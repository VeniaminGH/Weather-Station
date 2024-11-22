Overview
********

A Weather Station application to operate Weather Meter Kit
https://www.sparkfun.com/products/15901 from nucleo_wl55jc LoRaWAN board with
proprietary version of SparkFun Weather Shield https://www.sparkfun.com/products/13956.

Building and Running
********************

Before building the app, make sure to select the correct region in the
``prj.conf`` file.

The following commands build and flash the sample.

.. zephyr-app-commands::
   :zephyr-app: Weather-Station/app
   :board: wst_nucleo_wl55jc.overlay
   :goals: build flash
   :compact:

Extended Configuration
**********************

This application can be configured to run the application-layer clock
synchronization service and/or the remote multicast setup service
in the background.

The following commands build and flash the sample with clock synchronization
enabled.

.. zephyr-app-commands::
   :zephyr-app: Weather-Station/app
   :board: wst_nucleo_wl55jc.overlay
   :goals: build flash
   :gen-args: -DEXTRA_CONF_FILE=overlay-clock-sync.conf
   :compact:

The following commands build and flash the sample with remote multicast setup
enabled.

.. zephyr-app-commands::
   :zephyr-app: Weather-Station/app
   :board: wst_nucleo_wl55jc.overlay
   :goals: build flash
   :gen-args: -DEXTRA_CONF_FILE=overlay-multicast.conf
   :compact:
