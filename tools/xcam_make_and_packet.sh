#!/bin/bash

chip=$1

if [ "${chip}" = "t40" ] || [ "${chip}" = "t21" ] || [ "${chip}" = "t31" ] || [ "${chip}" = "t41" ]
then
	echo ----start make!----
	./xcam_make.sh
	./pack.py pack_confs/${chip}_conf_xcam_dev_snipe.json ${chip}
else
	echo ----Chip model is not supported for the time being!----
fi



