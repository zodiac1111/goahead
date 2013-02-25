json数据格式示例 {#json-date-example}
==============

列举前后端交互时的数据格式.用于调试和对照.

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
