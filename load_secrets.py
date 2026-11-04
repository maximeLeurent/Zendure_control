"""
PlatformIO pre-build script used to load secrets from a KeePassXC database.

This script is automatically executed by PlatformIO before compiling the
ESP32 firmware.

Its purpose is to keep sensitive information such as Wi-Fi passwords,
API keys, and authentication tokens outside of the source code and Git
repository.

During a normal build, the script:
1. Prompts the user for the KeePass master password.
2. Reads the configured entries from the KeePass database using keepassxc-cli.
3. Injects a Base64-encoded JSON registry into the PlatformIO build environment.
4. Does not write the retrieved secrets to any file.

IDE metadata generation skips secret loading because it does not compile or
upload the firmware.

The injected values can then be used directly from the C++ code, for example:

    WiFi.begin(get_secret("WIFI_SSID", false), get_secret("WIFI_PASSWORD", false));

Requirements:
- KeePassXC must be installed.
- keepassxc-cli must be available in the system PATH.
- The script must be declared as a PlatformIO pre-build script:

    extra_scripts =
        pre:load_secrets.py
"""

Import("env")

import subprocess
import getpass
import ssl
import base64
import json


def load_secrets():
    # -----------------------------------------------------------------------
    # Configuration
    # -----------------------------------------------------------------------

    # Path to the KeePass database.
    KEEPASS_DB = r"C:\Users\J0545844\Documents\secrets_esp32.kdbx"

    # Mapping between secret names, KeePass entry paths, and certificate flags.
    #
    # Example:
    #   WIFI_PASSWORD -> {"entry": "ESP32/WIFI_PASSWORD", "certificate": False}
    #
    # The value stored in the "Password" field of each KeePass entry will be
    # encoded in the secret registry used by get_secret().
    SECRETS = {
        "WIFI_SSID": {"entry": "WIFI_SSID", "certificate": False},
        "WIFI_PASSWORD": {"entry": "WIFI_PASSWORD", "certificate": False},
        "ENPHASE_SERIAL_NUMBER": {"entry": "ENPHASE_SERIAL_NUMBER", "certificate": False},
        "ENPHASE_USERNAME": {"entry": "ENPHASE_USERNAME", "certificate": False},
        "ENPHASE_PASSWORD": {"entry": "ENPHASE_PASSWORD", "certificate": False},
        "ENPHASE_ROOT_CA": {"entry": "ENPHASE_ROOT_CA", "certificate": True},
        "ZENDURE_SERIAL_NUMBER": {"entry": "ZENDURE_SERIAL_NUMBER", "certificate": False},
    }


    # -----------------------------------------------------------------------
    # KeePass authentication
    # -----------------------------------------------------------------------

    # Ask for the KeePass master password without displaying it in the terminal.
    master_password = getpass.getpass("KeePass master password: ")


    # -----------------------------------------------------------------------
    # KeePass secret retrieval
    # -----------------------------------------------------------------------

    def get_keepass_secret(entry):
        """
        Retrieve the Password field of a KeePass entry.

        The KeePass master password is passed to keepassxc-cli through stdin
        and is therefore not included in the command-line arguments.
        """

        result = subprocess.run(
            [
                "keepassxc-cli",
                "show",
                "-s",
                "-a", "Password",
                "-q",
                KEEPASS_DB,
                entry,
            ],
            input=master_password + "\n",
            text=True,
            encoding="utf-8",
            capture_output=True,
        )

        # Stop the build if the database cannot be opened or the entry
        # cannot be found.
        if result.returncode != 0:
            print(result.stderr)
            raise RuntimeError(
                f"Unable to read KeePass entry: {entry}"
            )

        return result.stdout[:-1] if result.stdout.endswith("\n") else result.stdout


    def parse_certif(value, secret_name):
        """Normalize and validate a PEM certificate or certificate bundle.

        Convert literal newline escapes and Windows line endings to LF, and ensure
        a trailing newline. The value must contain complete PEM certificates without
        a C++ raw-string wrapper. This checks parsing, not trust or expiration.

        Return the normalized PEM text. Raise RuntimeError if parsing fails,
        identifying the secret by name without exposing its contents.
        """
        certificate = (
            value.replace("\\r\\n", "\n")
            .replace("\\n", "\n")
            .replace("\r\n", "\n")
            .replace("\r", "\n")
            .strip() + "\n"
        )
        try:
            ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT).load_verify_locations(cadata=certificate)
        except (ssl.SSLError, ValueError):
            raise RuntimeError(
                f"Invalid certificate {secret_name}: store the complete PEM certificate in KeePass, "
                "including BEGIN CERTIFICATE and END CERTIFICATE, "
                "without the C++ raw-string wrapper."
            ) from None
        return certificate


    # -----------------------------------------------------------------------
    # PlatformIO build environment
    # -----------------------------------------------------------------------

    secret_values = {}
    for secret_name, settings in SECRETS.items():

        # Retrieve the secret from KeePass.
        secret = get_keepass_secret(settings["entry"])

        if settings["certificate"]:
            secret = parse_certif(secret, secret_name)
        if "\0" in secret:
            raise RuntimeError(f"Secret contains an unsupported NUL character: {secret_name}")
        secret_values[secret_name] = secret

    encoded_secrets = base64.b64encode(
        json.dumps(secret_values, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
    ).decode("ascii")
    env.Append(CPPDEFINES=[("KEEPASS_SECRETS_BASE64", f'\\"{encoded_secrets}\\"')])


    # Do not print the secret values.
    print("KeePass secrets successfully loaded.")


if env.IsIntegrationDump():
    print("Skipping KeePass secret loading for IDE metadata generation.")
else:
    load_secrets()