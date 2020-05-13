﻿/* mutation.c */
extern bool gain_mutation(player_type *creature_ptr, MUTATION_IDX choose_mut);
extern bool lose_mutation(player_type *creature_ptr, MUTATION_IDX choose_mut);
extern void lose_all_mutations(player_type *creature_ptr);
extern int calc_mutant_regenerate_mod(player_type *creature_ptr);
extern bool exe_mutation_power(player_type *creature_ptr, int power);
extern void become_living_trump(player_type *creature_ptr);
