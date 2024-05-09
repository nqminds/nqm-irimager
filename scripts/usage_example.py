import datetime
import logging
from nqm.irimager import IRImager, Logger
import numpy as np
from time import sleep

logging.basicConfig()
logging.getLogger().setLevel(1) # trace
logger = Logger()
# Your XML config,
# see http://documentation.evocortex.com/libirimager2/html/Overview.html#subsec_overview_config_file
filename_80 = '382x288@80Hz.xml'
filename_27 = '382x288@27Hz.xml'
filename_32 = '764x480@32Hz.xml'
filename_1000 = '72x56@1000Hz.xml'
XML_CONFIG = f"../tests/__fixtures__/{filename_80}"
irimager = IRImager(XML_CONFIG)
with irimager:
    start_time = datetime.datetime.now()
    previous_timestamp = start_time
    print(f"Started at {start_time}")
    while True: # press CTRL+C to stop this program
        try:
          array, timestamp = irimager.get_frame()
          # np.save(f"{timestamp}.npy", array)
        except Exception as error:
          print(f"error caught: {error}")
          if 'IRIMAGER' in repr(error):
            continue # if IRIMAGER in error message, just continue ignoring error, as sometimes spurious errors are raised
          else:
            print(f"Stopped at {datetime.datetime.now()}")
            raise error
        frame_in_celsius = array / (10 ** irimager.get_temp_range_decimal()) - 100
        # print(f"At {timestamp}: Average temperature is {frame_in_celsius.mean()}")
        time_interval = timestamp - previous_timestamp
        print(1/time_interval.total_seconds())
        previous_timestamp = timestamp

del logger # to stop
