const names = [
    {
        "name":"Ali BENTAOUET1",
        "id":7
    },
    {
        "name":"Ali BENTAOUET2",
        "id":2
    },
    {
        "name":"Ali BENTAOUET3",
        "id":9
    },
    {
        "name":"Mohamdi BICHILLA",
        "id":6
    },
    {
        "name": "youssra",
        "id":50
    },
{
    "name": "Yassine",
    "id":76
},
{
    "name": "marwa",
    "id":100
},
{
    "name": "Aymane",
    "id":56
},
{
    "name": "wissal",
    "id":79
},
,
{
    "name": "aya",
    "id":80
},
,
{
    "name": "adnane",
    "id":32
},
,
{
    "name": "reda",
    "id":90
}
]

const url = 'https://shpserver.pythonanywhere.com/allVariables';
const log = console.log
const outsidetemp = document.getElementById("outside").getElementsByTagName("span")[0]
const outsidehum =document.getElementById("outside").getElementsByTagName("span")[1]
const insidetemp = document.getElementById("inside").getElementsByTagName("span")[0]
const insidehum =document.getElementById("inside").getElementsByTagName("span")[1]
const Moisture= document.getElementById("Moisture")
const Water= document.getElementById("Water")
const smokeO = document.getElementById("outsideSmoke").getElementsByTagName("span")[0]
const smokeI = document.getElementById("insideSmoke").getElementsByTagName("span")[0]
const occupants = document.getElementById("occupants").getElementsByTagName("tbody")[0]
const Occupied = document.getElementById("Door").getElementsByTagName("h1")[0]
log(Occupied)
let InRoom = []
function addRow(obj){
    const tr = document.createElement('tr');
    const td1 = document.createElement('td');
    const td2 = document.createElement('td');
    const th = document.createElement('th');
    occupants.appendChild(tr)
    th.innerHTML = obj.id
    td1.innerHTML = obj.name
    td2.innerHTML = obj.time
    tr.appendChild(td1)
    tr.appendChild(th)
    tr.appendChild(td2)
}
// Get the current date and time
/* const currentDateTime = new Date();

// Format the date and time as strings
const formattedDate = currentDateTime.toLocaleDateString();
const formattedTime = currentDateTime.toLocaleTimeString();

// Display the date and time using alert
alert('Today\'s Date: ' + formattedDate + '\nCurrent Time: ' + formattedTime);
 

alert(date);
*/


const icon = {
    "rain" : document.getElementById("rain"),
    "sunny" :document.getElementById("sunny"),
    "storm": document.getElementById("storm")
}

function displayWeatherIcon(iconString){
    icon.rain.style.display= "none";
    icon.storm.style.display= "none";
    icon.sunny.style.display= "none";
    icon[iconString].style.display= "block"

}   


const fetchData = async () => {
  try {
    const response = await fetch(url, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json', // Set the content type according to your server's requirements
      },
      body: JSON.stringify({
        // Your data object goes here
      }),

    });
    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    const data = await response.json();
    
    updateWeather(data)
    updateAgri(data)
    updateSmoke(data)
    InRoom.splice(0, InRoom.length);
    occupants.innerHTML =""
    fillInRoom(data)
    if(data.someoneInRoom){
        Occupied.innerHTML = "Occupied"
    }
    else{
        Occupied.innerHTML = "Not Occupied"
    }
    for( let i in InRoom)
    {addRow(InRoom[i])}
    //Call update funtions:



    return data;
  } catch (error) {
    console.error('Error:', error.message);
  }
};

function updateWeather(data) {
    outsidetemp.innerHTML = data.outsideRoomTemperature.toFixed(1);
    outsidehum.innerHTML = data.outsideRoomHumidity.toFixed(1);
    insidetemp.innerHTML = data.insideRoomTemperature.toFixed(1);
    insidehum.innerHTML  =  data.insideRoomHumidity != null ?data.insideRoomHumidity.toFixed(1): 0 ;
    let rain = parseInt(data.rainDetection)
    if(rain <2000){
        displayWeatherIcon("storm") 
    }else if(rain>=2000 && rain <=3000){
        displayWeatherIcon("rain")
        
    }else if(rain>=3700 && rain <=4095){
        displayWeatherIcon("sunny")

    }

}
function updateAgri(data) {
    const value = (data.waterLevel/40.95).toFixed(2);
    Water.getElementsByTagName("span")[0].innerHTML = value > 9 ?`${value}%`:" ";

    Water.style.width = `${value}%`

    const valueM = ((2295 - (data.soilMoisture-1800))/31).toFixed(2);
    Moisture.getElementsByTagName("span")[0].innerHTML = valueM > 9 ?`${valueM}%`:" ";

    Moisture.style.width = `${valueM}%`
}


function updateSmoke(data) {
    smokeO.innerHTML = `${(data.outsideRoomSmoke/120.95).toFixed(2)} %`
    smokeI.innerHTML = `${(data.insideRoomSmoke/120.95).toFixed(2)} %`
}

setInterval(fetchData, 1000);
//fetchData()

function fillInRoom(data) {

    for (let i in data.occupants){
      
        let name = "unknown";
        for(let j in names)
        {
            if(names[j].id == data.occupants[i].id){
                name = names[j].name
            }
        }
        InRoom.push({
            "name": String(name),
            "id":String(data.occupants[i].id),
            "time":String(data.occupants[i].time).slice(0,8)
        })
    }


}