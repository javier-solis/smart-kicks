const TuftMedical= [42.349566, -71.064458];
const HarvardStadium = [42.366754, -71.126513];


const mainAddr = "http://608dev-2.net/sandbox/sc/team44/map/";
const serverFileMain = "main.py";
const serverFileLandmarks = "landmarks.py";

//======================

// == Helper functions ==

function getParamValue(param){
  return new URLSearchParams(window.location.search).get(param);
}

function getJSONGroup(array, key, value) {
  return array.filter((object) => {
      return object[key] === value;
  })[0];
};


// == Map Setup Stuff ==

async function getLandmarks() {
  return fetch('http://608dev-2.net/sandbox/sc/team44/map/get_landmarks.py')
  .then(res => res.json());
}

function getOrigin() {
  let lat_lon = getJSONGroup(landmarks, "name", "Lobby 7");
  return [lat_lon.lat, lat_lon.lon];;
}


// Retrieving Specific User Data

const username = getParamValue("user")

async function userTrail() {
  const query = "?trail-map="+username;
  const postURL = mainAddr+serverFileMain+query;

  const response = await fetch(postURL);

  if(response.status===200){ //succesfull GET
    const jsonResult = await response.json();
    document.getElementById("currentUser").innerHTML = username;
    return [jsonResult.locations, jsonResult.timing];
  }
}

let landmarks;
let map;



async function getPrevDestination() {
  return fetch('http://608dev-2.net/sandbox/sc/team44/map/main.py?destination='+username)
  .then(res => res.text());
}



async function main() {
  landmarks = await getLandmarks();

  const origin = getOrigin();

  map = L.map('map', {
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
  
  const userData = await userTrail();

  const markerRadius = 0.5;
  const pathColor="blue";

  // polyline
  the_line = L.polyline(userData[0], { color: pathColor }).addTo(map);
  the_line.bindPopup(`Trail Line: ${username}`);

  // markers
  userData[0].forEach((LatLng, i) =>{
    let circle = L.circle(LatLng, markerRadius).addTo(map);
    circle.setStyle({color: pathColor});
    circle.bindPopup(`Data Point: ${username}, ${userData[1][i]}`);
  });

  // last user destination

  let prevDestination = await getPrevDestination();
  updateDescription(prevDestination);

};


main()
.then(() => {landmarkMain()})