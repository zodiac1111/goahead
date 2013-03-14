json数据格式示例 {#json-date-example}
==============

列举前后端交互时的数据格式.用于调试和对照.
<http://jsoneditoronline.org/>

## 系统参数

	{
	    "meter_num": 23,
	    "sioplan_num": 12,
	    "monitor_ports": 4,
	    "netports_num": 3,
	    "sioports_num": 7,
	    "control_ports": 6
	}

## 时间同步

	{
	    "timestamp": "1361495376"
	}

## 串口方案

	{
	    "parity": [
	        "无",
	        "奇",
	        "偶"
	    ],
	    "data": [
	        7,
	        8,
	        9
	    ],
	    "stop": [
	        0,
	        1
	    ],
	    "baud": [
	        300,
	        600,
	        1200,
	        2400,
	        4800,
	        9600
	    ],
	    "commtype": [
	        "异步asyn",
	        "同步syn"
	    ],
	    "item": [
	        {
	            "no": "0",
	            "parity": "0",
	            "data": "7",
	            "stop": "1",
	            "baud": "3",
	            "commtype": "1"
	        },
	        {
	            "no": "1",
	            "parity": "0",
	            "data": "7",
	            "stop": "0",
	            "baud": "0",
	            "commtype": "0"
	        },
	        {
	            "no": "2",
	            "parity": "0",
	            "data": "7",
	            "stop": "0",
	            "baud": "0",
	            "commtype": "0"
	        },
	        {
	            "no": "3",
	            "parity": "2",
	            "data": "7",
	            "stop": "0",
	            "baud": "0",
	            "commtype": "0"
	        },
	        {
	            "no": "4",
	            "parity": "1",
	            "data": "7",
	            "stop": "0",
	            "baud": "0",
	            "commtype": "0"
	        },
	        {
	            "no": "5",
	            "parity": "0",
	            "data": "7",
	            "stop": "0",
	            "baud": "0",
	            "commtype": "0"
	        }
	    ]
	}

## 网口方案

	{
	    "eth_num": "3",
	    "item": [
	        {
	            "no": "0",
	            "eth": "2",
	            "ip": "192.168.001.189",
	            "mask": "255.255.255.000",
	            "gateway": "192.168.001.002"
	        },
	        {
	            "no": "1",
	            "eth": "2",
	            "ip": "192.168.001.183",
	            "mask": "255.255.255.000",
	            "gateway": "030.000.000.000"
	        },
	        {
	            "no": "2",
	            "eth": "1",
	            "ip": "192.168.001.182",
	            "mask": "255.255.255.000",
	            "gateway": "100.010.000.000"
	        }
	    ]
	}

##存储周期

	{
	    "cycle": [
	        "1分钟",
	        "5分钟",
	        "10分钟",
	        "15分钟",
	        "30分钟",
	        "60分钟",
	        "120分钟",
	        "180分钟",
	        "360分钟",
	        "480分钟",
	        "720分钟",
	        "1440分钟"
	    ],
	    "item": [
	        {
	            "en": "0",
	            "t": "0"
	        },
	        {
	            "en": "1",
	            "t": "2"
	        },
	        {
	            "en": "0",
	            "t": "0"
	        },
	        {
	            "en": "0",
	            "t": "4"
	        },
	        {
	            "en": "0",
	            "t": "0"
	        },
	        {
	            "en": "0",
	            "t": "0"
	        }
	    ]
	}

#表计参数

	{
	    "portplan": [
	        "0",
	        "1",
	        "2"
	    ],
	    "port": [
	        "485-1",
	        "485-2",
	        "485-3",
	        "485-4",
	        "COM5",
	        "COM6",
	        "COM7"
	    ],
	    "procotol": [
	        "645_97",
	        "645_07",
	        "weisheng",
	        "Mk3",
	        "Mk6",
	        "IEC1107",
	        "SL7000",
	        "Email",
	        "ABBAlpha",
	        "645_97ZF",
	        "sx102",
	        "HL102",
	        "GX102",
	        "hb102",
	        "df102",
	        "BJ102s",
	        "Dl719ss",
	        "C61850",
	        "376.1",
	        "lje-dlms",
	        "gs102",
	        "dd",
	        "nx102",
	        "nm376",
	        "elster-d",
	        "mbap",
	        "sd102",
	        "cw102",
	        "cw102zf"
	    ],
	    "factory": [
	        "华立",
	        "威盛",
	        "兰吉尔",
	        "红相",
	        "other"
	    ],
	    "type": [
	        "3相3线",
	        "3相4线"
	    ],
	    "items": [
	        {
	            "mtrno": "0",
	            "iv": "0",
	            "line": "000001",
	            "addr": "000000000120",
	            "pwd": "01233123",
	            "port": "0",
	            "portplan": "0",
	            "protocol": "2",
	            "factory": "1",
	            "ph_wire": "1",
	            "it_dot": "3",
	            "xl_dot": "3",
	            "v_dot": "3",
	            "i_dot": "1",
	            "p_dot": "2",
	            "q_dot": "1",
	            "ue": "2",
	            "ie": "0"
	        },
	        {
	            "mtrno": "1",
	            "iv": "0",
	            "line": "000001",
	            "addr": "000000000120",
	            "pwd": "00000000",
	            "port": "0",
	            "portplan": "0",
	            "protocol": "2",
	            "factory": "0",
	            "ph_wire": "1",
	            "it_dot": "0",
	            "xl_dot": "3",
	            "v_dot": "3",
	            "i_dot": "1",
	            "p_dot": "2",
	            "q_dot": "1",
	            "ue": "2",
	            "ie": "0"
	        },
	        {
	            "mtrno": "2",
	            "iv": "0",
	            "line": "000001",
	            "addr": "000000000120",
	            "pwd": "00000000",
	            "port": "0",
	            "portplan": "0",
	            "protocol": "2",
	            "factory": "0",
	            "ph_wire": "1",
	            "it_dot": "0",
	            "xl_dot": "3",
	            "v_dot": "0",
	            "i_dot": "1",
	            "p_dot": "0",
	            "q_dot": "0",
	            "ue": "0",
	            "ie": "0"
	        }
	    ]
	}

#配置信息

	{
	    "info_webbin": "/mnt/nor/bin/webs",
	    "info_webconf": "/mnt/nor/conf/goahead.conf",
	    "info_weblog": "/mnt/nor/goahead.log",
	    "info_rtuconf": "/mnt/nor/conf",
	    "info_rtupara": "/mnt/nor/para",
	    "info_wwwroot": "/mnt/nor/www",
	    "major": "1",
	    "minor": "1",
	    "patchlevel": "248",
	    "git_version": "r248-ad22eb2"
	}

#其他
(无)
