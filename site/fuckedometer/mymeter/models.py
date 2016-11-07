from django.db import models
from django.contrib.auth.models import User

from urllib import request
import re

class Device(models.Model):
    serial_number = models.IntegerField(unique=True)
    device_id = models.CharField(max_length=50, unique=True, verbose_name="Device ID")
    owner = models.ForeignKey(User)
    data_source = models.ForeignKey('DataSource', null=True, help_text='You can specify what data your device will display here')

    def __str__(self):
        return '{:s}\'s Meter (#{:d})'.format(self.owner.first_name or self.owner.username, self.serial_number)

class DataSourceException(Exception):
    pass

class DataSource(models.Model):
    PUBLIC = 'pub'
    PRIVATE = 'pri'
    PRIVACY_CHOICES = (
            (PUBLIC, 'Public'),
            (PRIVATE, 'Private'),
            )

    name = models.CharField(max_length=100, help_text='Give your data source a friendly name so you can find it later')
    poll_url = models.URLField(verbose_name='Poll URL', help_text='The URL here should return a text/plain document containing a single number between 0 and 100')
    creator = models.ForeignKey(User)
    privacy = models.CharField(max_length=3, choices=PRIVACY_CHOICES, default=PRIVATE, help_text='If you mark this data source Public, all users will be able to choose it as an option for their meter.')
    fail_count = models.IntegerField(default=0)

    value_re = re.compile(r'^(\d+)')

    def privacy_label(self):
        if self.privacy == self.PRIVATE:
            return ' (Private)'
        else:
            return ''

    def reset_failed(self):
        if self.fail_count != 0:
            self.fail_count = 0
            self.save()

    def update(self):
        if self.fail_count > 10:
            print("Not updating data source {:s}: too many failures".format(self))
            return
        try:
            with request.urlopen(self.poll_url) as response:
                data_filter = self.value_re.match(response.read().decode('utf-8'))

                if data_filter:
                    self.new_reading(data_filter.group(1))
                    self.reset_failed()
                else:
                    raise DataSourceException("Could not parse number from data source")
        except Exception:
            self.fail_count += 1
            self.save()
            raise

    def new_reading(self, value):
        reading = MeterReading()
        reading.data_source = self
        reading.value = value
        reading.save()

    def __str__(self):
        return '{:s}{:s}'.format(self.name, self.privacy_label())

class MeterReading(models.Model):
    read_date = models.DateTimeField(auto_now_add=True)
    value = models.IntegerField()
    data_source = models.ForeignKey(DataSource, related_name='readings')

    class Meta:
        ordering = ['-read_date']

    def __str__(self):
        return "{:s}: {:d}".format(self.read_date, self.value)

class Setting(models.Model):
    default_data_source = models.ForeignKey(DataSource)

