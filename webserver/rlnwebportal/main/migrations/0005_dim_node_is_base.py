# Generated by Django 3.2.3 on 2021-06-27 12:42

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('main', '0004_alter_dim_node_status_status'),
    ]

    operations = [
        migrations.AddField(
            model_name='dim_node',
            name='is_base',
            field=models.BooleanField(default=0),
            preserve_default=False,
        ),
    ]