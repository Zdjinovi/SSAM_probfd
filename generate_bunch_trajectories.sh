mkdir -p trajectories_of_different_problems

for ((i=1; i<=588; i=i+1))
do 
	timeout 30m ./fast-downward.py /home/research/zihao.deng/thorsten_ecai/benchmarks/generators/elevators/p$i.pddl --search "ilao_fret(costs=maxprob(), eval=ppdbs(patterns=classical_generator(generator=systematic(pattern_max_size=2), subcollection_finder_factory=multiplicative_max_orthogonality_factory())), trajectories=1, random_seed=3, trajectory_length=100)"
	mv trajectory_0.plan ./trajectories_of_different_problems/trajectory_$i.plan
done	
