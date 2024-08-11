import shutil
Import("env")
pipotype=env["PIOENV"]


if(not "buildfs" in COMMAND_LINE_TARGETS):
  print("Do not copy default config file")
  exit(0)

if pipotype == "analog":
  shutil.copyfile('configs/analog_config.json', 'data/config/default.json')
elif pipotype == "motion":
  shutil.copyfile('configs/motion_config.json', 'data/config/default.json')
elif pipotype == "range":
  shutil.copyfile('configs/range_config.json', 'data/config/default.json')
else:
  print("Do not copy default config file")

