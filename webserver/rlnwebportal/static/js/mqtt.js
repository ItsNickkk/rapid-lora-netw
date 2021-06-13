var mqtt;
var reconnectTimeout = 2000;
var host;
var port;
var topic = "RLN/gateway";
function MQTTsetParam(hosts, ports){
	host = hosts;
	port = ports;
}
function onFailure(message) {
	console.log("Connection Attempt to Host "+host+"Failed");
	setTimeout(MQTTconnect, reconnectTimeout);
}
function onMessageArrived(msg){	
	// out_msg = "Message received "+msg.payloadString+"<br>";
	// out_msg = out_msg+"Message received Topic "+msg.destinationName;
	item = JSON.parse(msg.payloadString);
	if(typeof item['update'] == 'undefined'){ //This condition does not contain 'update' key
		console.log(msg.payloadString);
		tfkey = ["dg", "hg", "wt", "fa"];
		for(i = 0; i<tfkey.length; i++){
			if(typeof item[tfkey[i]] == "undefined" || item[tfkey[i]] == 0){
				item[tfkey[i]] = false;
			}
			else{
				item[tfkey[i]] = true;
			}
		}
		$.ajax({
			url: "api/manageEntries",
			dataType:'json',
			data:{
				"latest_id": ""
			},
			success: function(data){     
				id = data['data'][0]['latest_id'];
				buildArray(item, id);
			}
		});
	}
	else{//This condition contain 'update' key
		if(!msg.payloadString==selfUpdateJSON){
			var findID = item["id"];
			res = ongoing_arr.find(obj => obj["id"] = findID);
			res["status"] = item["update"]
			index = ongoing_arr.findIndex(obj => obj["id"] = findID);
			ongoing_arr[index] = res;
		}
	}
	makePending();
}

function onConnect() {
	console.log("Connected ");
	mqtt.subscribe(topic);
	// message = new Paho.MQTT.Message("Hello World");
	// message.destinationName = topic;
	// mqtt.send(message);
}
function MQTTconnect() {
	console.log("connecting to "+ host +" "+ port);
	var x=Math.floor(Math.random() * 10000); 
	var cname="orderform-"+x;
	mqtt = new Paho.MQTT.Client("192.168.0.22",9001,cname);
	var options = {
		timeout: 3,
		onSuccess: onConnect,
		onFailure: onFailure,
		};
	mqtt.onMessageArrived = onMessageArrived

	mqtt.connect(options); //connect
}

function MQTTsend(msg){
	message = new Paho.MQTT.Message(msg);
	message.destinationName = topic;
	mqtt.send(message);
}

function buildArray(item, id){
	currentdate = new Date(); 
	var datetime =  currentdate.getDate() + "/"
					+ (currentdate.getMonth()+1)  + "/" 
					+ currentdate.getFullYear() + " "
					+ currentdate.getHours() + ":"  
					+ currentdate.getMinutes() + ":" 
					+ currentdate.getSeconds();
	var post = {
		id: id,
		name: item['n'],
		address: item['add'],
		datetime: datetime,
		status: "Incomplete",
		nodeid: JSON.stringify(nodelist[item['node']]),
		hpno: item['hp'],
		occupants: item['cnt'],
		danger: item['dg'],
		hug: item['hg'],
		water: item['wt'],
		firstaid: item['fa'],
	}
	ongoing_arr.unshift(post);
}