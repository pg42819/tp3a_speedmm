intel_vars_script=/share/apps/intel/parallel_studio_xe_2019/compilers_and_libraries_2019/linux/bin/compilervars.sh
if [ -f ${intel_vars_script} ]; then
  echo "Sourcing intel env scripts at $intel_vars_script"
  source ${intel_vars_script} intel64
else
  echo "Could not find the intel env scripts at $intel_vars_script"
fi

