
# NOTICE: Please do not remove the '#' before 'PBS'

# Name of your job
#PBS -N darxan

# Declaring job as not re-runnable
#PBS -r n

# Resource allocation (how many nodes? how many processes per node?)
#PBS -l nodes=4:ppn=12

# Max execution time of your job (hh:mm:ss)
# Debug cluster max limit: 00:05:00 
# Batch cluster max limit: 00:30:00
# Your job may got killed if you exceed this limit
#PBS -l walltime=00:01:00

cd $PBS_O_WORKDIR
mpiexec ./ex22 48 In_48_65 shortest_path 11 # edit this line to fit your needs!

