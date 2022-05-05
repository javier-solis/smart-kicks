import json

landmarks_file = '/var/jail/home/team44/server/map/landmarks.json'

def request_handler(request):
    if request["method"] == "GET":
        f = open(landmarks_file)
        return json.dumps(json.load(f))

    elif request["method"] == "POST":
        return "You cannot perform a POST here."
    else:
        return "Error."