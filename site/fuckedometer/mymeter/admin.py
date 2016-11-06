from django.contrib import admin
from mymeter.models import Device, MeterReading, DataSource, Setting
from django.db.models import Q

class DataSourceAdmin(admin.ModelAdmin):
    USER_EDITABLE_FIELDS = (
            'name',
            'poll_url',
            'privacy',
            )
    def get_form(self, request, obj=None, **kwargs):
        if not request.user.is_superuser:
            kwargs['fields'] = self.USER_EDITABLE_FIELDS

        return super(DataSourceAdmin, self).get_form(request, obj, **kwargs)

    def get_queryset(self, request):
        qs = super(DataSourceAdmin, self).get_queryset(request)
        if request.user.is_superuser:
            return qs
        return qs.filter(creator=request.user)

    def has_change_permission(self, request, obj=None):
        if request.user.is_superuser:
            return True
        if not obj:
            return True
        return obj.creator == request.user

    def has_delete_permission(self, request, obj=None):
        return self.has_change_permission(request, obj)

    def save_model(self, request, instance, form, change):
        if not change:
            instance.creator = request.user
        super(DataSourceAdmin, self).save_model(request, instance, form, change)

admin.site.register(DataSource, DataSourceAdmin)

class DeviceAdmin(admin.ModelAdmin):
    USER_EDITABLE_FIELDS = (
            'data_source',
            )

    def get_form(self, request, obj=None, **kwargs):
        if not request.user.is_superuser:
            kwargs['fields'] = self.USER_EDITABLE_FIELDS

        return super(DeviceAdmin, self).get_form(request, obj, **kwargs)

    def get_queryset(self, request):
        qs = super(DeviceAdmin, self).get_queryset(request)
        if request.user.is_superuser:
            return qs
        return qs.filter(owner=request.user)

    def formfield_for_foreignkey(self, db_field, request, **kwargs):
        if db_field.name == 'data_source':
            kwargs['queryset'] = DataSource.objects.filter(creator=request.user)
        return super(DeviceAdmin, self).formfield_for_foreignkey(db_field, request, **kwargs)
    def has_delete_permission(self, request, obj=None):
        return request.user.is_superuser

admin.site.register(Device, DeviceAdmin)
admin.site.register(MeterReading)
admin.site.register(Setting)
