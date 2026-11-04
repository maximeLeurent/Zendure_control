Gateway test 1/7: GET https://192.168.1.103/ivp/meters
HTTP status: 200
[
    {
        "eid": 704643328,
        "state": "enabled",
        "measurementType": "production",
        "phaseMode": "three",
        "phaseCount": 1,
        "meteringStatus": "normal",
        "statusFlags": []
    },
    {
        "eid": 704643584,
        "state": "enabled",
        "measurementType": "net-consumption",
        "phaseMode": "three",
        "phaseCount": 1,
        "meteringStatus": "normal",
        "statusFlags": []
    }
]

Gateway test 2/7: GET https://192.168.1.103/ivp/meters/readings
HTTP status: 200
[
    {
        "eid": 704643328,
        "timestamp": 1792699764,
        "actEnergyDlvd": 13312920.174,
        "actEnergyRcvd": 15712.104,
        "apparentEnergy": 16313534.441,
        "reactEnergyLagg": 3878744.596,
        "reactEnergyLead": 14.383,
        "instantaneousDemand": -0.783,
        "activePower": -0.783,
        "apparentPower": 120.295,
        "reactivePower": 108.627,
        "pwrFactor": 0.0,
        "voltage": 236.861,
        "current": 0.508,
        "freq": 50.0,
        "channels": [
            {
                "eid": 1778385169,
                "timestamp": 1792699764,
                "actEnergyDlvd": 13312920.174,
                "actEnergyRcvd": 15712.104,
                "apparentEnergy": 16313534.441,
                "reactEnergyLagg": 3878744.596,
                "reactEnergyLead": 14.383,
                "instantaneousDemand": -0.783,
                "activePower": -0.783,
                "apparentPower": 120.295,
                "reactivePower": 108.627,
                "pwrFactor": 0.0,
                "voltage": 236.861,
                "current": 0.508,
                "freq": 50.0
            },
            {
                "eid": 1778385170,
                "timestamp": 1792699764,
                "actEnergyDlvd": 0.0,
                "actEnergyRcvd": 25022.34,
                "apparentEnergy": 4073.423,
                "reactEnergyLagg": 2491.343,
                "reactEnergyLead": 1.408,
                "instantaneousDemand": -0.012,
                "activePower": -0.012,
                "apparentPower": 0.111,
                "reactivePower": -0.0,
                "pwrFactor": 0.0,
                "voltage": 1.565,
                "current": 0.071,
                "freq": 50.0
            },
            {
                "eid": 1778385171,
                "timestamp": 1792699764,
                "actEnergyDlvd": 0.099,
                "actEnergyRcvd": 61616.729,
                "apparentEnergy": 40.206,
                "reactEnergyLagg": 2512.105,
                "reactEnergyLead": 10.845,
                "instantaneousDemand": 0.0,
                "activePower": 0.0,
                "apparentPower": 0.0,
                "reactivePower": 0.0,
                "pwrFactor": 0.0,
                "voltage": 12.275,
                "current": 0.0,
                "freq": 50.0
            }
        ]
    },
    {
        "eid": 704643584,
        "timestamp": 1792699764,
        "actEnergyDlvd": 10787742.857,
        "actEnergyRcvd": 9846654.649,
        "apparentEnergy": 24570962.785,
        "reactEnergyLagg": 350408.341,
        "reactEnergyLead": 8166889.277,
        "instantaneousDemand": 1447.099,
        "activePower": 1447.099,
        "apparentPower": 1511.599,
        "reactivePower": -426.142,
        "pwrFactor": 0.961,
        "voltage": 237.117,
        "current": 6.374,
        "freq": 50.0,
        "channels": [
            {
                "eid": 1778385425,
                "timestamp": 1792699764,
                "actEnergyDlvd": 10787742.857,
                "actEnergyRcvd": 9846654.649,
                "apparentEnergy": 24570962.785,
                "reactEnergyLagg": 350408.341,
                "reactEnergyLead": 8166889.277,
                "instantaneousDemand": 1447.099,
                "activePower": 1447.099,
                "apparentPower": 1511.599,
                "reactivePower": -426.142,
                "pwrFactor": 0.961,
                "voltage": 237.117,
                "current": 6.374,
                "freq": 50.0
            },
            {
                "eid": 1778385426,
                "timestamp": 1792699764,
                "actEnergyDlvd": 0.0,
                "actEnergyRcvd": 63015.083,
                "apparentEnergy": 14787.299,
                "reactEnergyLagg": 2543.633,
                "reactEnergyLead": 0.0,
                "instantaneousDemand": -0.087,
                "activePower": -0.087,
                "apparentPower": 0.423,
                "reactivePower": 0.0,
                "pwrFactor": 0.0,
                "voltage": 3.915,
                "current": 0.109,
                "freq": 50.0
            },
            {
                "eid": 1778385427,
                "timestamp": 1792699764,
                "actEnergyDlvd": 0.0,
                "actEnergyRcvd": 106699.588,
                "apparentEnergy": 4821.938,
                "reactEnergyLagg": 2805.686,
                "reactEnergyLead": 0.023,
                "instantaneousDemand": -0.059,
                "activePower": -0.059,
                "apparentPower": 0.138,
                "reactivePower": 0.014,
                "pwrFactor": 0.0,
                "voltage": 1.222,
                "current": 0.112,
                "freq": 50.0
            }
        ]
    },
    {
        "eid": 1023410688,
        "timestamp": 1792699755,
        "actEnergyDlvd": 0.0,
        "actEnergyRcvd": 0.0,
        "apparentEnergy": 0.0,
        "reactEnergyLagg": 0.0,
        "reactEnergyLead": 0.0,
        "instantaneousDemand": 0.0,
        "activePower": 0.0,
        "apparentPower": 0.0,
        "reactivePower": 0.0,
        "pwrFactor": 0.0,
        "voltage": 0.0,
        "current": 0.0,
        "freq": 0.0,
        "channels": [
            {
                "eid": 2097152513,
                "timestamp": 1792699755,
                "actEnergyDlvd": 0.0,
                "actEnergyRcvd": 0.0,
                "apparentEnergy": 0.0,
                "reactEnergyLagg": 0.0,
                "reactEnergyLead": 0.0,
                "instantaneousDemand": 0.0,
                "activePower": 0.0,
                "apparentPower": 0.0,
                "reactivePower": 0.0,
                "pwrFactor": 0.0,
                "voltage": 0.0,
                "current": 0.0,
                "freq": 0.0
            },
            {
                "eid": 2097152258,
                "timestamp": 1792699755,
                "actEnergyDlvd": 0.0,
                "actEnergyRcvd": 0.0,
                "apparentEnergy": 0.0,
                "reactEnergyLagg": 0.0,
                "reactEnergyLead": 0.0,
                "instantaneousDemand": 0.0,
                "activePower": 0.0,
                "apparentPower": 0.0,
                "reactivePower": 0.0,
                "pwrFactor": 0.0,
                "voltage": 0.0,
                "current": 0.0,
                "freq": 0.0
            },
            {
                "eid": 2097152515,
                "timestamp": 1792699755,
                "actEnergyDlvd": 0.0,
                "actEnergyRcvd": 0.0,
                "apparentEnergy": 0.0,
                "reactEnergyLagg": 0.0,
                "reactEnergyLead": 0.0,
                "instantaneousDemand": 0.0,
                "activePower": 0.0,
                "apparentPower": 0.0,
                "reactivePower": 0.0,
                "pwrFactor": 0.0,
                "voltage": 0.0,
                "current": 0.0,
                "freq": 0.0
            }
        ]
    }
]

Gateway test 3/7: GET https://192.168.1.103/api/v1/production
HTTP status: 200
{
  "wattHoursToday": 9512,
  "wattHoursSevenDays": 27092,
  "wattHoursLifetime": 13312920,
  "wattsNow": -1
}


Gateway test 4/7: GET https://192.168.1.103/ivp/pdm/energy
HTTP status: 200
{
    "production": {
        "pcu": {
            "wattHoursToday": 9613,
            "wattHoursSevenDays": 27319,
            "wattHoursLifetime": 5119866,
            "wattsNow": 0
        },
        "rgm": {
            "wattHoursToday": 0,
            "wattHoursSevenDays": 0,
            "wattHoursLifetime": 0,
            "wattsNow": 0
        },
        "eim": {
            "wattHoursToday": 9512,
            "wattHoursSevenDays": 27092,
            "wattHoursLifetime": 13312920,
            "wattsNow": -1
        }
    },
    "consumption": {
        "eim": {
            "wattHoursToday": 19820,
            "wattHoursSevenDays": 0,
            "wattHoursLifetime": 14253611,
            "wattsNow": 1462
        }
    }
}

Gateway test 5/7: GET https://192.168.1.103/api/v1/production/inverters
HTTP status: 200
[
  {
    "serialNumber": "122212099001",
    "lastReportDate": 1792687139,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 252
  },
  {
    "serialNumber": "122212093947",
    "lastReportDate": 1792687116,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 260
  },
  {
    "serialNumber": "122212110499",
    "lastReportDate": 1792687064,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 256
  },
  {
    "serialNumber": "122212110427",
    "lastReportDate": 1792686987,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 256
  },
  {
    "serialNumber": "122213000726",
    "lastReportDate": 1792687099,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 257
  },
  {
    "serialNumber": "122212109862",
    "lastReportDate": 1792687128,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 258
  },
  {
    "serialNumber": "122213000188",
    "lastReportDate": 1792686427,
    "devType": 1,
    "lastReportWatts": 0,
    "maxReportWatts": 257
  }
]


Gateway test 6/7: GET https://192.168.1.103/ivp/livedata/status
HTTP status: 200
{
    "info": "Last available status data shown. Live stream not enabled. Enableusing /ivp/livedata/stream/ API",
    "connection": {
        "mqtt_state": "connected",
        "prov_state": "configured",
        "auth_state": "ok",
        "sc_stream": "disabled",
        "sc_debug": "disabled"
    },
    "meters": {
        "last_update": 1792699777,
        "soc": 0,
        "main_relay_state": 1,
        "gen_relay_state": 5,
        "backup_bat_mode": 1,
        "backup_soc": 10,
        "is_split_phase": 0,
        "phase_count": 1,
        "enc_agg_soc": 0,
        "enc_agg_energy": 0,
        "acb_agg_soc": 0,
        "acb_agg_energy": 0,
        "pv": {
            "agg_p_mw": 0,
            "agg_s_mva": 119640,
            "agg_p_ph_a_mw": 0,
            "agg_p_ph_b_mw": 0,
            "agg_p_ph_c_mw": 0,
            "agg_s_ph_a_mva": 119640,
            "agg_s_ph_b_mva": 0,
            "agg_s_ph_c_mva": 0
        },
        "storage": {
            "agg_p_mw": 0,
            "agg_s_mva": -14076,
            "agg_p_ph_a_mw": 0,
            "agg_p_ph_b_mw": 0,
            "agg_p_ph_c_mw": 0,
            "agg_s_ph_a_mva": -14076,
            "agg_s_ph_b_mva": 0,
            "agg_s_ph_c_mva": 0
        },
        "grid": {
            "agg_p_mw": 1471276,
            "agg_s_mva": 1537154,
            "agg_p_ph_a_mw": 1471276,
            "agg_p_ph_b_mw": 0,
            "agg_p_ph_c_mw": 0,
            "agg_s_ph_a_mva": 1537154,
            "agg_s_ph_b_mva": 0,
            "agg_s_ph_c_mva": 0
        },
        "load": {
            "agg_p_mw": 1471276,
            "agg_s_mva": 1642718,
            "agg_p_ph_a_mw": 1471276,
            "agg_p_ph_b_mw": 0,
            "agg_p_ph_c_mw": 0,
            "agg_s_ph_a_mva": 1642718,
            "agg_s_ph_b_mva": 0,
            "agg_s_ph_c_mva": 0
        },
        "generator": {
            "agg_p_mw": 0,
            "agg_s_mva": 0,
            "agg_p_ph_a_mw": 0,
            "agg_p_ph_b_mw": 0,
            "agg_p_ph_c_mw": 0,
            "agg_s_ph_a_mva": 0,
            "agg_s_ph_b_mva": 0,
            "agg_s_ph_c_mva": 0
        }
    },
    "tasks": {
        "task_id": -1846806806,
        "timestamp": 1792689356
    },
    "counters": {
        "main_CfgLoad": 2,
        "main_CfgChanged": 1,
        "main_taskUpdate": 2,
        "MqttClient_respond": 4,
        "MqttClient_msgarrvd": 2,
        "MqttClient_reconnect": 1,
        "MqttClient_create": 29,
        "MqttClient_setCallbacks": 29,
        "MqttClient_connect": 29,
        "MqttClient_connect_err": 18,
        "MqttClient_connect_Err": 18,
        "MqttClient_subscribe": 11,
        "SSL_Keys_Create": 29,
        "rest_Status": 1453
    },
    "dry_contacts": {
        "�\u000B": {
            "dry_contact_id": "�\u000B",
            "dry_contact_type": "�\u000B",
            "dry_contact_load_name": "\u0006",
            "dry_contact_status": 0
        },
        "": {
            "dry_contact_id": "",
            "dry_contact_type": "",
            "dry_contact_load_name": "",
            "dry_contact_status": 0
        },
        "": {
            "dry_contact_id": "",
            "dry_contact_type": "",
            "dry_contact_load_name": "",
            "dry_contact_status": 0
        },
        "": {
            "dry_contact_id": "",
            "dry_contact_type": "",
            "dry_contact_load_name": "",
            "dry_contact_status": 0
        }
    }
}

Gateway test 7/7: GET https://192.168.1.103/ivp/meters/reports/consumption
HTTP status: 200
[
    {
        "createdAt": 1792699781,
        "reportType": "total-consumption",
        "cumulative": {
            "currW": 1468.486,
            "actPower": 1468.486,
            "apprntPwr": 1653.710,
            "reactPwr": -315.947,
            "whDlvdCum": 14253848.648,
            "whRcvdCum": 0.000,
            "varhLagCum": -3528336.752,
            "varhLeadCum": 8166876.815,
            "vahCum": 24570969.696,
            "rmsVoltage": 236.800,
            "rmsCurrent": 6.984,
            "pwrFactor": 0.89,
            "freqHz": 50.00
        },
        "lines": [
            {
                "currW": 1468.486,
                "actPower": 1468.486,
                "apprntPwr": 1653.710,
                "reactPwr": -315.947,
                "whDlvdCum": 14253848.648,
                "whRcvdCum": 0.000,
                "varhLagCum": -3528336.752,
                "varhLeadCum": 8166876.815,
                "vahCum": 24570969.696,
                "rmsVoltage": 236.800,
                "rmsCurrent": 6.984,
                "pwrFactor": 0.89,
                "freqHz": 50.00
            }
        ]
    },
    {
        "createdAt": 1792699781,
        "reportType": "net-consumption",
        "cumulative": {
            "currW": 1469.993,
            "actPower": 1469.993,
            "apprntPwr": 1533.722,
            "reactPwr": -424.292,
            "whDlvdCum": 941261.188,
            "whRcvdCum": 0.000,
            "varhLagCum": 350408.341,
            "varhLeadCum": 8166891.198,
            "vahCum": 24570969.696,
            "rmsVoltage": 236.800,
            "rmsCurrent": 6.477,
            "pwrFactor": 0.96,
            "freqHz": 50.00
        },
        "lines": [
            {
                "currW": 1469.993,
                "actPower": 1469.993,
                "apprntPwr": 1533.722,
                "reactPwr": -424.292,
                "whDlvdCum": 941261.188,
                "whRcvdCum": 0.000,
                "varhLagCum": 350408.341,
                "varhLeadCum": 8166891.198,
                "vahCum": 24570969.696,
                "rmsVoltage": 236.800,
                "rmsCurrent": 6.477,
                "pwrFactor": 0.96,
                "freqHz": 50.00
            }
        ]
    }
]
All gateway API tests completed. No further measurement requests will be sent until restart.
