from __future__ import absolute_import

from celery import shared_task
from mymeter.models import DataSource

@shared_task
def update_data(data_source_id):
    DataSource.objects.get(id=data_source_id).update()

