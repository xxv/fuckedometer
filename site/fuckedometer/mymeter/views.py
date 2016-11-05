from django.shortcuts import render, get_object_or_404
from django.http import HttpResponse

from mymeter.models import Device, MeterReading, DataSource, Setting
from mymeter.tasks import update_data

import json

def index(request):
    device = get_device(request)

    if device and device.data_source:
        data_source = device.data_source
    else:
        data_source = get_default_data_source()
    value = get_latest(data_source).value
    return HttpResponse("{:d}\n".format(value))

def data_source(request, data_source_id):
    data_source = get_object_or_404(DataSource, pk=data_source_id)
    readings = list(map(lambda d: d.value, data_source.readings.filter()[:10]))

    return HttpResponse(json.dumps(readings))

def get_default_data_source():
    data_source = None

    setting = Setting.objects.get(pk=1)
    if setting:
        data_source = setting.default_data_source
    else:
        data_source = DataSource.objects.get(pk=1)

    return data_source


def update(request):
    for data_source in DataSource.objects.all():
        update_data.delay(data_source.id)

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

def get_latest(data_source):
    return MeterReading.objects.filter(data_source=data_source)[0]

