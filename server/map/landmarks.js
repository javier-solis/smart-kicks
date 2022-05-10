let previous;

function updateDescription(name){
  let all = getJSONGroup(landmarks, "name", name);
  document.getElementById("currentDestination").innerHTML = name + "<br>" + all.description;
}

function onClick() {

    const postURL = mainAddr+serverFileLandmarks;
    if(previous){
        previous.setStyle({color: 'red'});
    }
    previous=this;

    this.setStyle({color: 'purple'});

    const landmarkName = this.getPopup().getContent();

    updateDescription(landmarkName);

    fetch(postURL, {
        method: 'POST',
        headers: {
          "Content-type": "application/x-www-form-urlencoded; charset=UTF-8"
        },
        body: `user=${username}&landmark_name=${landmarkName}`
      })
      .then(res => res.text())
      .then(res => {
          console.log("Successful POSTed! Got back this message:", res);
      })
      .catch(err => {
        console.log('Failed to POST. Error message:', err);
      });
  
}

function landmarkMain() {

  for(let index=0; index < landmarks.length; index++){
  
      let popupText = landmarks[index].name
  
      let coord = [landmarks[index].lat, landmarks[index].lon];
  
      const landmark = L.circle(coord, 10).bindPopup(popupText).addTo(map).on('click', onClick);
      landmark.setStyle({color: 'red'});
  }
}



