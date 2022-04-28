let previous;

function onClick() {

    const postURL = mainAddr+serverFileLandmarks;
    if(previous){
        previous.setStyle({color: 'red'});
    }
    previous=this;

    this.setStyle({color: 'purple'});

    const landmarkName = this.getPopup().getContent();

    fetch(postURL, {
        method: 'POST',
        headers: {
          "Content-type": "application/x-www-form-urlencoded; charset=UTF-8"
        },
        body: `user=${username}&landmark_name=${landmarkName}`
      })
      .then(res.text())
      .then(res => {
          console.log("Successful POSTed! Got back this message:", res);
      })
      .catch(err => {
        console.log('Failed to POST. Error message:', err);
      });
      
}


const landmarksArray = locations.landmarks;

for(let index=0; index<landmarksArray.length; index++){

    let popupText = landmarksArray[index].name

    let coord = [landmarksArray[index].lat, landmarksArray[index].lon];

    const landmark = L.circle(coord, 10).bindPopup(popupText).addTo(map).on('click', onClick);
    landmark.setStyle({color: 'red'});
}
