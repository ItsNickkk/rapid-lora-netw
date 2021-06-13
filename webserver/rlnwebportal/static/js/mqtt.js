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
		makePending();
	}
	else{//This condition contain 'update' key
		if(msg.payloadString!=selfUpdateJSON){
			
			var findID = item["id"];
			res = findEntry(ongoing_arr, findID);
			if(typeof res == "undefined"){
				res = findEntry(completed_arr, findID);
			}
			res[0]["status"] = item["update"];
			if(res[0]["status"] == 3){
				ongoing_arr.splice(res[1], 1);
				completed_arr.push(res[0]);
			}
			else{
				completed_arr.splice(res[1], 1);
				ongoing_arr.push(res[0]);
			}
			makePending();
			// if(res[0]["status"] != 3){
			// 	ongoing_arr[index] = res;
			// }
			// else{
			// 	ongoing_arr.splice(index, 1);
			// 	completed_arr.push(res);
			// }
			// makePending();
		}
	}
	
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

function findEntry(arr, findID){
	for (var i=0; i < arr.length; i++) {
		if (arr[i].id == findID) {
			return [arr[i],i];
		}
	}
}