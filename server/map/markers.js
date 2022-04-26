let shorthand = locations.landmarks

let previous;

function onClick() {
    if(previous){
        previous.setStyle({color: 'red'});
    }
    previous=this;

    this.setStyle({color: 'purple'});

    // const landmarkName = this.getPopup().getContent();

    // fetch(landmarkUpdateURL, {
    //     method: 'POST',
    //     headers: {
    //       "Content-type": "application/x-www-form-urlencoded; charset=UTF-8"
    //     },
    //     body: `user=${user}&landmark=${landmarkName}`
    //   })
    //   .then(res.text())
    //   .then(res => {
    //       console.log("Successful POST! Got back this message:", res);
    //   })
    //   .catch(err => {
    //     console.log('Failed to POST. Error message:', err);
    //   });
      
}


for(let index=0; index<shorthand.length; index++){

    let popupText = shorthand[index].name

    let coord = [shorthand[index].lat, shorthand[index].lon];

    landmark = L.circle(coord, 10).bindPopup(popupText).addTo(map).on('click', onClick); // for now, they all have a radius of 15s
    landmark.setStyle({color: 'red'});
}
