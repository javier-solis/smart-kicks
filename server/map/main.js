const TuftMedical= [42.34956664091039, -71.06445822683442];
const HarvardStadium = [42.36675464638942, -71.12651381366604];

function getParam(param){
  return new URLSearchParams(window.location.search).get(param);
}

const user = getParam("user") // gotten from url 
const mainUrl = "http://608dev-2.net/sandbox/sc/team44/server/map/main.py";

let locations = {
  "landmarks": [{
      "name": "Lobby 7",
      "lat": 42.3591871,
      "lon": -71.0931501,
      "description": "Idk 1"
      },
      {
      "name": "Lobby 10",
      "lat": 42.35949364379263,
      "lon": -71.09190495832806,
      "description": "Idk 2"
      } 
  ]    
}

function getJSONGroup(array, key, value) {
  return array.filter((object) => {
      // console.log(object)
      return object[key] === value;
  })[0];
};

let origin = (()=>{
      let stuff = getJSONGroup(locations.landmarks, "name", "Lobby 7")
      console.log(stuff)
      return [stuff.lat, stuff.lon];
    })();

// == Map Setup Stuff == 
// Main Config
let map = L.map('map', {
    maxZoom: 19,
    minZoom: 15,
    maxBounds: [
        TuftMedical, // south west
        HarvardStadium, // north east
        ],
        maxBoundsViscosity: 1.0
    }).setView(origin, 18);

// OSM Tiles
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 20,
    maxNativeZoom: 20,
attribution: '&copy; <a href="https://openstreetmap.org/copyright">OpenStreetMap Contributors</a>'
}).addTo(map);

// show the scale bar on the lower left corner
L.control.scale({imperial: true, metric: true}).addTo(map);



// retreive specific user data
async function userTrail(usr) {
  const userQuery = "?trail-map="+usr;
  console.log(mainUrl)
  const response = await fetch(mainUrl+userQuery);

  if(response.status===200){ //succesfull GET

    const jsonResult = await response.json();

    document.getElementById("current-user").innerHTML = usr;

    return [jsonResult.locations, jsonResult.timing];
  }
}



async function main() {
  const userData = await userTrail(user);
  console.log(userData)

  const marker_radius = 0.5;
  const colorChosen="blue"

  // Add a polyline connecting their data
  the_line = L.polyline(userData[0], { color: colorChosen }).addTo(map);
  the_line.bindPopup(`Trail Line: ${user}`);

  userData[0].forEach( (LatLng, i) =>{
    let circle = L.circle(LatLng, marker_radius).addTo(map);
    circle.setStyle({color: colorChosen});
    circle.bindPopup(`Data Point: ${user} @ ${userData[1][i]}`);

    // If we wanted squares instead:
    // var lineMark = L.rectangle(circle.getBounds()).addTo(map);
    // lineMark.setStyle({fillColor: 'blue', fillOpacity: 1, color: 'blue'});
    });

};


main();