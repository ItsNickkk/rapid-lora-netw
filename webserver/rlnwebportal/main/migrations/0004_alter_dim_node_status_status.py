# Generated by Django 3.2.3 on 2021-05-28 08:10

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('main', '0003_dim_node_node_desc'),
    ]

    operations = [
        migrations.AlterField(
            model_name='dim_node_status',
            name='status',
            field=models.CharField(max_length=50),
        ),
    ]