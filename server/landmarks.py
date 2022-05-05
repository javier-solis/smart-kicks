import json

landmarks_file = '/var/jail/home/team44/landmarks.json'

def request_handler(request):
    f = open(landmarks_file)
    return json.dumps(json.load(f))