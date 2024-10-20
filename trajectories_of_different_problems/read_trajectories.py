import re

# Regular expression to match Atom and NegatedAtom
atom_pattern = re.compile(r'(Atom|NegatedAtom) ([\w\-]+)\((.*?)\)')
# Regular expression to match actions
action_pattern = re.compile(r'\(([\w\-]+ [\w\s,\[\]]+)\)$')

# Function to parse atoms and actions from a given string
def parse_atoms_and_actions(input_string):
    # Find all atom matches
    atom_matches = atom_pattern.findall(input_string)

    # Parse out the state atoms
    state_atoms = []
    for match in atom_matches:
        atom_type, predicate, args = match
        args_list = [arg.strip() for arg in args.split(',')]
        state_atoms.append({
            'type': atom_type,
            'predicate_name': predicate,
            'args': args_list
        })

    # Find the action
    action_match = action_pattern.search(input_string)

    # Parse out the action
    action = None
    if action_match:
        action_args = action_match.group(1)
        action_parts = action_args.split()
        # Filter out '[outcome', '0]', '1]', '[outcome,', '0', '1'
        action = {
            'action_name': action_parts[0],
            'args': [arg for arg in action_parts[1:] if not re.match(r'\[?outcome|\d+\]?', arg)]
        }

    return state_atoms, action

def get_all_boundedl(f,a):

	types_of_f_args = [ o[0] for o in f['args']]
	types_of_a_args = [ o[0] for o in a['args']]
	type_maps = [ [ i for i,o in enumerate(types_of_a_args) if o in types ] for types in types_of_f_args]

	import itertools
	combs = list(itertools.product(*type_maps))

	Ls = [ (a['action_name'], f['predicate_name'], comb) for comb in combs]
	Ls2 = [ (a['action_name'], f['predicate_name']+'_neg', comb) for comb in combs]
	Lset = set(Ls)
	Lset.update(Ls2)
	return Lset

############################### read the trajectories:
K = 588
traces = []
for i in range(1,K):
	file_str = "trajectory_"+str(i)+".plan"

	try: 
		f = open(file_str, "r")
		lines = []
		for x in f:
			lines.append(x)
		f.close()

		states = []
		actions = [] # the last action of each trajectory is a dummy action

		for line in lines:
			s, a = parse_atoms_and_actions(line)
			states.append(s)
			actions.append(a)

		traces.append({'states':states, 'actions':actions})
	except FileNotFoundError:
		pass 	


lifted_f_set = set()
for tr in traces:
	for state, a in zip(tr['states'], tr['actions']):
		for f in state:
			if a['action_name']!='unit':
				L_set = get_all_boundedl(f,a)
				lifted_f_set.update(L_set)

lifted_f_list = []
for f in lifted_f_set:
   lifted_f_list.append(f)
lifted_f_list = sorted(lifted_f_list)

lifted_f_to_idx = {}
for i,f in enumerate(lifted_f_list):
    lifted_f_to_idx[f]=i


for f in lifted_f_list:
	print(f)


#################################### get lifted states:
lifted_states = []

for tr in traces:
	stps = list(zip(tr['states'], tr['actions']))
	for i, sa in enumerate(stps):
		state = sa[0]
		action = sa[1]
		
		if action['action_name']=='move-right-gate' and i+1 < len(tr['states']):
			next_state = stps[i+1][0]

			lifted_st0 = [None] * len(lifted_f_list)
			lifted_st1 = [None] * len(lifted_f_list)

			for f in state:
				evalcurr = True if f['type']=='Atom' else False
				
				mp = [-1] * len(f['args'])

				for i_f, o_f in enumerate(f['args']):
					for i_a, o_a in enumerate(action['args']):
						if o_f == o_a:
							mp[i_f] = i_a
							break

				if (action['action_name'], f['predicate_name'], tuple(mp)) in lifted_f_to_idx:
					lifted_st0[ lifted_f_to_idx[(action['action_name'], f['predicate_name'], tuple(mp))]] = evalcurr
					lifted_st0[ lifted_f_to_idx[(action['action_name'], f['predicate_name']+'_neg', tuple(mp))]] = not evalcurr
	

			for f in next_state:
				evalnext = True if f['type']=='Atom' else False
				
				mp = [-1] * len(f['args'])

				for i_f, o_f in enumerate(f['args']):
					for i_a, o_a in enumerate(action['args']):
						if o_f == o_a:
							mp[i_f] = i_a
							break

				if (action['action_name'], f['predicate_name'], tuple(mp)) in lifted_f_to_idx:
					lifted_st1[ lifted_f_to_idx[(action['action_name'], f['predicate_name'], tuple(mp))]] = evalnext
					lifted_st1[ lifted_f_to_idx[(action['action_name'], f['predicate_name']+'_neg', tuple(mp))]] = not evalnext		

			lifted_states.append([lifted_st0, lifted_st1])


for st in lifted_states:
	print(st[0])
	print(st[1])
	print()

##################################### construct the tensors:

################ construct tensor T1
import torch
N = len(lifted_states[0][0])

shape = (N,N,N)
T1 = torch.zeros(shape)

def dfs_genMtrxIdx(d, curr_idx_set, st):
    if d==0:
        collect.append(curr_idx_set)
        return

    for i in range(N):
        if st[0][i]==False and st[1][i]==True:
            dfs_genMtrxIdx(d-1, curr_idx_set+[i],st)
    return

N_tran = len(lifted_states)
degree = 3
for s in range(N_tran):
    state = lifted_states[s]
    collect = []
    dfs_genMtrxIdx(degree,[],state)
    # print(collect)
    for idx in collect:
        T1[tuple(idx)]+=1

print(N)
print(N_tran)
print(sum(sum(sum(T1))))
# print(m)

########## construct tensor T0
N = len(lifted_states[0][0])

shape = (N,N,N)
T0 = torch.zeros(shape)

def dfs_genMtrxIdx_preNeg(d, curr_idx_set, st):
    if d==0:
        collect_preNeg.append(curr_idx_set)
        return

    for i in range(N):
        if st[0][i]==False:
            dfs_genMtrxIdx_preNeg(d-1, curr_idx_set+[i],st)
    return

N_tran = len(lifted_states)
degree = 3
for s in range(N_tran):
    state = lifted_states[s]
    collect_preNeg = []
    dfs_genMtrxIdx_preNeg(degree,[],state)
    # print(collect)
    for idx in collect_preNeg:
        T0[tuple(idx)]+=1

print(N)
print(N_tran)
print(sum(sum(sum(T0))))











  
