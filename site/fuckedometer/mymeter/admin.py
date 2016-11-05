from django.contrib import admin
from mymeter.models import Device, MeterReading, DataSource

admin.site.register(Device)
admin.site.register(MeterReading)
admin.site.register(DataSource)
