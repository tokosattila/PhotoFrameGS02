import os
import platform

Import("env")

if platform.system().lower() == "windows":
  try:
    tJobs = int(os.getenv("PIO_BUILD_JOBS", "1"))
  except ValueError:
    tJobs = 1

  env.SetOption("num_jobs", max(1, tJobs))
  print("[limit_jobs] PlatformIO num_jobs set to {}".format(max(1, tJobs)))
