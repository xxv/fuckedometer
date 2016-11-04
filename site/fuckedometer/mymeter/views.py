from django.shortcuts import render, get_object_or_404
from django.http import HttpResponse

from mymeter.models import Device, MeterReading

def index(request):
    device_id = None
    if 'id' in request.GET:
        device_id = request.GET['id']
    device = get_object_or_404(Device, device_id=device_id)

    return HttpResponse("{:d}\n".format(get_latest(device).value))

def get_latest(device):
    return MeterReading.objects.filter(device=device)[0]

