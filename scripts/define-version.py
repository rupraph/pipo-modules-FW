import shutil
Import("env")
import subprocess

def getVersion():
  tags = subprocess.check_output(['git', 'show-ref', '--tags']).decode().split('\n')
  tags = list(map(lambda x: x.split(' '), tags))
  head = subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode().strip()
  for tag in tags:
    if len(tag[0]) == 0:
      continue
    chash, name = tag
    if head in chash:
      return name.split('/')[2]
  return head[:10]
version = getVersion()
print("Pipo software version: ", version)
env.Append(CPPDEFINES=[
  ("PIPO_FW_VERSION", version)
])