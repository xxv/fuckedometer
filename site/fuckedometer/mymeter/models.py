from django.db import models
from django.contrib.auth.models import User

class Device(models.Model):
    serial_number = models.IntegerField(unique=True)
    device_id = models.CharField(max_length=50, unique=True, verbose_name="Device ID")
    owner = models.ForeignKey(User)
    poll_url = models.URLField(blank=True)

    def __str__(self):
        #return '#{:d} {:s} ({:s})'.format(self.serial_number, self.owner, self.device_id)
        return '#{:d} ({:s})'.format(self.serial_number, self.device_id)

class MeterReading(models.Model):
    device = models.ForeignKey(Device)
    read_date = models.DateTimeField(auto_now_add=True)
    value = models.IntegerField()
    poll_url = models.URLField()

    class Meta:
        ordering = ['-read_date']

    def __str__(self):
        return "{:s}: {:d}".format( self.read_date, self.value)
        #return "{:s} {:s}: {:d}".format(self.device, self.read_date, self.value)

