copy Debug64\Release\*.dll ..\obs-studio\debug64\rundir\Release\obs-plugins\64bit
rem copy Debug64\Release\*.pdb ..\obs-studio\debug64\rundir\Release\obs-plugins\64bit
copy Debug32\Release\*.dll ..\obs-studio\debug32\rundir\Release\obs-plugins\32bit
rem copy Debug32\Release\*.pdb ..\obs-studio\debug32\rundir\Release\obs-plugins\32bit

copy data\locale\*.ini ..\obs-studio\debug64\rundir\Release\data\obs-plugins\window-follower\locale
copy data\locale\*.ini ..\obs-studio\debug32\rundir\Release\data\obs-plugins\window-follower\locale

pause