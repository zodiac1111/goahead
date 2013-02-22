json数据格式示例 {#json-date-example}
==============

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
