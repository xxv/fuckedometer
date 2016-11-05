from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^$', views.index, name='index'),
    url(r'^/update$', views.update, name='update'),
    url(r'^/data_source/(?P<data_source_id>[0-9]+)$', views.data_source, name='data_source')
]
