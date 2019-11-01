#include "rwfile.h"
#include "point.h"
#include "dist.h"
#include "avl.h"
#include "queue.h"
#include "draw.h"

void set_new_sensor(TPoint* selected_target,Queue* sensor_queue){

	if (sensor_queue != NULL){
		QueueEntry *queue_iterator = sensor_queue->head;
		AVLTree *avl_tree = avl_tree_new((AVLTreeCompareFunc) point_compare);
		while (queue_iterator != NULL) {
			TPoint* sensor = (TPoint*)(queue_iterator->data);

			avl_tree_insert(avl_tree,&(sensor->name), sensor);
			queue_iterator = queue_iterator->next;
		}
		selected_target->kind = K_Sensor;
		selected_target->aux = avl_tree;
	}
}

void set_cover(TPointFile* pf, int new_covered_target_max){
	pf->cover -= new_covered_target_max;
}

void set_sensor_new_communication(TPoint* selected_target, Queue* sensor_queue)
{
	if (sensor_queue != NULL){
		QueueEntry *queue_iterator = sensor_queue->head;

		while (queue_iterator != NULL) {
			TPoint* sensor = (TPoint*)(queue_iterator->data);
			AVLTree *avl_tree_sensor = sensor->aux;
			avl_tree_insert(avl_tree_sensor,&(selected_target->name), selected_target);
			queue_iterator = queue_iterator->next;
		}
	}
}

void set_target_new_capture_sensor(TPoint* selected_target, Queue* visited_target_queue){
	QueueEntry *queue_iterator = visited_target_queue->head;

	double point_x = selected_target->x;
	double point_y = selected_target->y;

	double target_x;
	double target_y;
	char * target_name;
	PKind target_kind;

	while (queue_iterator != NULL) {
		TPoint* target = (TPoint*)(queue_iterator->data);
		target_x = target->x;
	    target_y = target->y;
	    target_name = target->name;
	    target_kind = target->kind;
	    if( (point_x != target_x || point_y != target_y) && (target_kind == K_Target)){
			avl_tree_insert(target->aux,&(selected_target->name), selected_target);
	    }
	    queue_iterator = queue_iterator->next;
	}
}

void dist_to_well(TPointFile* pf,TPoint* selected_target, Queue* sensor_queue){

	double communication_radius = pf->communication_radius;

	TPoint* well_point = pf->points[0];
	double pos_well[2], pos_target[2];
	pos_well[0] = well_point->x;
	pos_well[1] = well_point->y;
	pos_target[0] = selected_target->x;
	pos_target[1] = selected_target->y;
	double dist_well_target = sqrt(dist_sq(pos_well,pos_target,2));

	if (dist_well_target < communication_radius){
		queue_push_head(sensor_queue, well_point);
	}
}

void maj_pf(TPointFile* pf,TPoint* selected_target, Queue* sensor_queue, Queue* visited_target_queue, int new_covered_target_max){

	// dist_to_well(pf,selected_target,sensor_queue);

	set_new_sensor(selected_target,sensor_queue);
	set_cover(pf,new_covered_target_max);
	set_sensor_new_communication(selected_target,sensor_queue);
	set_target_new_capture_sensor(selected_target,visited_target_queue);
}


void find_best_target(TPointFile* pf, TPoint* point, TPoint** selected_target, Queue** visited_target_queue, int* new_covered_target_max, AVLTree** visited_target_avl, int debug){

	if (debug){
		printf("### ON TRAITE ###\n");
		print_node(point);
		printf("comm_queue : \n");
		print_queue(point->communication_queue,print_node);
		printf("########################\n");
	}

	double communication_radius = pf->communication_radius;
	double capture_radius = pf->capture_radius;
	double point_x = point->x;
	double point_y = point->y;
	Queue *target_in_radius_queue = point->communication_queue;
	QueueEntry *queue_iterator = target_in_radius_queue->head;

	Queue *covered_target_in_radius_queue;
	QueueEntry *queue_iterator_2;

	int visit_node = 0;
	int new_covered_target;

	TPoint* covered_target_in_radius;
	double covered_target_in_radius_x;
	double covered_target_in_radius_y;
	char * covered_target_in_radius_name;
	PKind covered_target_in_radius_kind;
	AVLTree* covered_target_in_radius_aux;

	/* We iterate on all targets */
	while (queue_iterator != NULL) {
	    TPoint* target_in_radius = (TPoint*)(queue_iterator->data);
	    double target_in_radius_x = target_in_radius->x;
	    double target_in_radius_y = target_in_radius->y;
	    AVLTree* target_in_radius_aux = target_in_radius->aux;
	    char * target_in_radius_name = target_in_radius->name;
	    PKind target_in_radius_kind = target_in_radius->kind;
	    if( (point_x != target_in_radius_x || point_y != target_in_radius_y) && (target_in_radius_kind == K_Target)){

	    	if (debug){
		    	printf("### ON VISITE %d###\n",visited_target_avl == NULL ? 0 : avl_tree_num_entries(*visited_target_avl));
				print_node(target_in_radius);
				printf("########################\n");
			}

	    	visit_node = 0;
	    	new_covered_target = 0;
	    	/* If we use an AVL*/
	    	if(visited_target_avl != NULL){
	    		/* We look if we have already visited the current target */
	    		Queue *point_queue = avl_tree_lookup(*visited_target_avl,&(target_in_radius->name));

	    		/* We never have visited the current target */
	    		if (point_queue == NULL){
	    			/* We add the target in the avl (his name is the key), the
	    			 content his a stack in which we put the point*/
	    			Queue *queue = queue_new();
	    			queue_push_head(queue, point);

	    			avl_tree_insert(*visited_target_avl,&(target_in_radius->name), queue);
				}
				else{
					if (debug){
						printf("On poursuit PAS...\n");
						queue_push_head(point_queue, point);
					}
					visit_node = 1;
					if (debug){
						printf("QUEUE\n");
						print_queue(point_queue,print_node);
						printf("QUEUE\n");
					}
				}
	    	}
	    	if(visit_node==0){
	    		if (debug){
	    			printf("On poursuit ...\n");
	    		}
	    		Queue *covered_target_in_radius_queue = target_in_radius->capture_queue;
	    		QueueEntry *queue_iterator_2 = covered_target_in_radius_queue->head;
	    		if (debug){
	    			printf("~~~~~~~~~~~~~~~~\n");
	    		}
	    		while (queue_iterator_2 != NULL) {

	    			covered_target_in_radius = (TPoint*)(queue_iterator_2->data);

	    			covered_target_in_radius_x = covered_target_in_radius->x;
					covered_target_in_radius_y = covered_target_in_radius->y;
					covered_target_in_radius_name = covered_target_in_radius->name;
					covered_target_in_radius_kind = covered_target_in_radius->kind;
					covered_target_in_radius_aux = covered_target_in_radius->aux;
					if( (target_in_radius_x != covered_target_in_radius_x || target_in_radius_y != covered_target_in_radius_y) && (covered_target_in_radius_kind == K_Target)){

						if (debug){
							print_node(covered_target_in_radius);
						}

						if (avl_tree_num_entries(covered_target_in_radius_aux) == 0){

							if (debug){
								printf("=> new\n");
							}

	    					new_covered_target++;
	    				}
					}

	    			queue_iterator_2 = queue_iterator_2->next;
	    		}

	    		if (debug){
	    			printf("~~~~~~~~~~~~~~~~\n");
	    		}

	    		if(avl_tree_num_entries(target_in_radius_aux) == 0){

	    			new_covered_target++;
	    		}
	    		if (debug){
		    		printf("=> %d VOISINS / MAX COURANT : %d\n",new_covered_target,*new_covered_target_max);
	    		}

		    	if(new_covered_target >= *new_covered_target_max){

		    		*selected_target = target_in_radius;
		    		*visited_target_queue = covered_target_in_radius_queue;
		    		*new_covered_target_max = new_covered_target;

		    	}
	    	}
	    }
	    queue_iterator = queue_iterator->next;
  	}
}

void greedy_construction(TPointFile* pf){
	int debug = 0;
	if (debug || 1){
		printf("\n################################ GREED ################################\n");
	}
	if (debug){
		print_node(pf->points[5]);
		print_queue(pf->points[5]->communication_queue,print_node);
		printf("zzz\n");
		print_queue(pf->points[5]->capture_queue,print_node);
		printf("########################\n\n");
	}

	TPoint* well_point = pf->points[0];

	TPoint* selected_target;
	Queue* visited_target_queue;
	int new_covered_target_max = 0;

	TPoint* global_selected_target;
	Queue* global_visited_target_queue;
	int global_new_covered_target_max = 0;

	int i;
	AVLTree* visited_target_avl;

	find_best_target(pf, well_point, &selected_target, &visited_target_queue,&new_covered_target_max, NULL, debug);

	avl_tree_insert(pf->solution,&(well_point->name),well_point);
	avl_tree_insert(pf->solution,&(selected_target->name),selected_target);

	Queue *empty_queue = queue_new();

	queue_push_head(empty_queue,well_point);

	maj_pf(pf,selected_target,empty_queue,visited_target_queue,new_covered_target_max);

	if (debug){
		printf("######################## ON CHOISIT ########################\n");
		print_node(selected_target);
		printf("SOLUTION GLOBALE : \n");
		print_avl_tree(pf->solution,print_node);
		printf("########################\n\n");
	}

	Queue * qued = selected_target->capture_queue;
	QueueEntry *queue_iterator = qued->head;

	while(pf->cover > 0){
		// draw_data(pf);
		visited_target_avl = avl_tree_new((AVLTreeCompareFunc) point_compare);
		new_covered_target_max = 0;
		printf("cover : %d\n",pf->cover);
		AVLTree* current_solution_avl = pf->solution;
		AVLTreeValue* current_solution_list = avl_tree_to_array(current_solution_avl);
		int current_solution_list_length = avl_tree_num_entries(current_solution_avl);

		for(i = 0; i < current_solution_list_length; i++){
			TPoint* current_sensor = (TPoint*)(current_solution_list[i]);
			find_best_target(pf,current_sensor,&selected_target,&visited_target_queue,&new_covered_target_max,&visited_target_avl,debug);
		}
		avl_tree_insert(pf->solution,&(selected_target->name),selected_target);

		if (debug){
			printf("######################## ON CHOISIT ########################\n");
			print_node(selected_target);
			printf("SOLUTION GLOBALE : \n");
			print_avl_tree(pf->solution,print_node);
			printf("########################\n\n");
		}

		Queue *sensor_queue = avl_tree_lookup(visited_target_avl,&(selected_target->name));
		maj_pf(pf,selected_target,sensor_queue,visited_target_queue,new_covered_target_max);
	}

	if (debug){
		printf("AUX\n");
		int e;
		for (e = 0; e < pf->nbpoints; ++e)
		{
			printf("########## NODE ########\n");
			print_node(pf->points[e]);
			printf("### AUX ###\n");
			print_avl_tree(pf->points[e]->aux,print_node);
			printf("####################\n");
		}
		printf("AUX\n");
	}

	if (debug || 1){
		printf("RESULT : %d\n",avl_tree_num_entries(pf->solution));
		printf("###### FIN GREED ########\n");
	}

}