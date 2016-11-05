from django.shortcuts import render
from django.http import HttpResponse

from mymeter.models import Device, MeterReading, DataSource

def index(request):
    device = get_device(request)

    if device and device.data_source:
        value = get_latest(device).value
    else:
        value = 42
    return HttpResponse("{:d}\n".format(value))

def update(request):
    for data_source in DataSource.objects.all():
        data_source.update()

    return HttpResponse("")

def get_device(request):
    device = None
    if 'id' in request.GET:
        device_id = request.GET['id']
        try:
            device = Device.objects.get(device_id=device_id)
        except Device.DoesNotExist:
            pass

    return device

def get_latest(device):
    return MeterReading.objects.filter(data_source=device.data_source)[0]

