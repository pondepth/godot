@echo off
cd /d D:\Github\godot_psp
tar --exclude=.git --exclude=bin --exclude=.scons_cache --exclude=.godot -czf - . | ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL -i "%TEMP%\psp_vm_key" -p 2222 builder@127.0.0.1 "rm -rf /home/builder/godot && mkdir -p /home/builder/godot && tar -xzf - -C /home/builder/godot"
