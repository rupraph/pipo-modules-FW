import shutil
Import("env")
pipotype=env["PIOENV"]

toCopy=['analog', 'motion', 'range']
def copy():
  if(not "buildfs" in COMMAND_LINE_TARGETS):
    print("Do not copy default config file")
    return;
  if pipotype in toCopy:
    path= f"configs/{pipotype}_config.json"
    shutil.copyfile(path, 'data/default.json')
    shutil.copyfile(path, 'data/configs/default.json')
    print(f"Copy default config file {path}")
  else:
    print("Do not copy default config file")

copy()