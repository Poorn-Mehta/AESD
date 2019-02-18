/*
*	Custom Geologist Module by Poorn Mehta
*	Sorts and filters a list of predefined animals
*	Can accept 2 parameters: Animal Type Identity (string, case sensitive) and Number of Repetitions
*	Refernce Links:
*	http://linuxkernel51.blogspot.com/2011/03/use-of-module-parameters-in-kernel.html
*	https://blog.sourcerer.io/writing-a-simple-linux-kernel-module-d9dc3762c234
*	https://stackoverflow.com/questions/3467850/cross-compiling-a-kernel-module 
*	Note: I took help of my teammate Khalid AlAwadhi to learn about and implement linked lists,
*	since I was new to it. The help was restricted to syntax understanding and usage.
*/


// Includes

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/list.h>


// Variables

// Input
char *animal_id = "";
int min_reps = -1;

// Others
uint32_t Allocated_Space_Bytes_1 = 0;
uint32_t Allocated_Space_Bytes_2 = 0;
uint16_t node_counter_1 = 0;
uint16_t node_counter_2 = 0;
uint8_t i, j, k, Type_ID_Array_Length;
uint8_t match=0;
char swap[40];

struct timespec time_entry, time_exit, time_required; //For time


// Structure of Animals
// Contains Type_ID (string), Repetitions, and Kernel's own linked list
struct Animal_Type_ID
{
	char Type_ID[40];
	int Reps;	
	struct list_head list;
};

// Primary Two Sets
struct Animal_Type_ID Animals_Linked_List_1, Animals_Linked_List_2, *Add_Type_ID, *Temp_Type_ID;

// Some 50 Animals in Predefined Array
// TA Aakash Kumar has permitted this
char Preset_Type_ID_Array[][40] = {
					 "Babirusa", "Hellbender", "Dog", "Dog", "Rat", "Dog", "Fossa", "Frog", "Deer", "Babirusa",
					 "Cat", "Pig", "Hare", "Pig", "Fossa", "Horse", "Hare", "Fossa", "Deer", "Pig",
					 "Babirusa", "Horse", "Sloth", "Cat", "Dog", "Rat", "Fossa", "Fossa", "Frog", "Rat",
					 "Hellbender", "Babirusa", "Rat", "Dog", "Cat", "Hellbender", "Sloth", "Frog", "Deer", "Echidna",
					 "Horse", "Echidna", "Babirusa", "Cat", "Dog", "Dog", "Hellbender", "Narwhal", "Deer", "Frog"
					};

module_param(min_reps, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(min_reps, "Minimum Repetitions of Animal");
module_param(animal_id, charp, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(animal_id, "Type ID of Animal");

// Kernel Module insertion and processing
static int __init Custom_Geologist_Init(void)
{
	getnstimeofday(&time_entry); //Store the timestamp when module has started processing (installtion)
	
	printk("\n--->>>Starting Custom Geologist Module<<<---");

	// Calculating preset array's length
	Type_ID_Array_Length = sizeof(Preset_Type_ID_Array)/sizeof(Preset_Type_ID_Array[0]);
	printk("\nPreset Array Contains %d Animals", Type_ID_Array_Length);
	
	// Alphabetical Array Sort
	// Used reference code from this website: https://www.sanfoundry.com/c-program-sort-names-alphabetical-order/

	for(k = 0; k < (Type_ID_Array_Length - 1); k ++)
	{
		for(j = (k + 1); j < Type_ID_Array_Length; j++)
		{
			if(strcmp(Preset_Type_ID_Array[k], Preset_Type_ID_Array[j]) > 0)
			{
				strcpy(swap, Preset_Type_ID_Array[k]);
				strcpy(Preset_Type_ID_Array[k], Preset_Type_ID_Array[j]);
				strcpy(Preset_Type_ID_Array[j], swap);
			}
		}
	}

	printk("\nSorted Array of all Animal Types\n\n");
	
	for(k = 0; k < Type_ID_Array_Length; k ++)		printk("Animal Type ID: %s", Preset_Type_ID_Array[k]);

//	Linked List Head Initialization
	INIT_LIST_HEAD(&Animals_Linked_List_1.list);
	INIT_LIST_HEAD(&Animals_Linked_List_2.list);

	// Creating a linked list without duplication from preset array (while also recording repetition occurences)
	for(i = 0; i < Type_ID_Array_Length; ++ i)
	{
		// Matching animal type id using in built looping function
		// If a match is found (animal is already added and duplicate is found)
		// Then no need to loop further using list_for_each_entry
		// If animal is new, then create a node in linked list
		list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)
		{	
			if(strcmp(Temp_Type_ID->Type_ID, Preset_Type_ID_Array[i]) == 0)
			{
				printk("Duplticate Animal Type ID Entry Found of *%s*", Temp_Type_ID->Type_ID);
				Temp_Type_ID->Reps++;
				match = 1; 
				break;
			}
		}
		
		if(match == 0)
		{
			// Allocating memory for new node		
			Add_Type_ID = kmalloc(sizeof(*Add_Type_ID), GFP_KERNEL);

			// Updating the variable that keeps track of memory usage
			Allocated_Space_Bytes_1 += sizeof(*Add_Type_ID);

			// Adding new animal type id (create a node in linked list)
			strcpy(Add_Type_ID->Type_ID, Preset_Type_ID_Array[i]);
			Add_Type_ID->Reps = 1;
			INIT_LIST_HEAD(&Add_Type_ID->list);
			list_add_tail(&(Add_Type_ID->list), &(Animals_Linked_List_1.list));
		}

		match = 0;		//Resetting match variable
	}
	
	printk("\nSpace (bytes) used by Linked List 1: %d", Allocated_Space_Bytes_1);

	// Linked List 1 Output using inbuilt looping function
	printk("\n--->>>Linked List 1<<<---\n");
	list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)
	{
		printk("Animal Type ID: %s <-> Repetitions: %d", Temp_Type_ID->Type_ID, Temp_Type_ID->Reps);
		node_counter_1 += 1;
	}
	printk("\nLinked List 1 has %d Nodes\n", node_counter_1);
	
	// Case handling for combination of input parameters from user
	// 1 - No Parameters
	// 2 - Only animal type id is passed
	// 3 - Only required minimum of repetitions is passed
	// 4 - Both - animal type id and minimum number of repetitions are passed
	// Procedure: Filter from Linked List 1, On match - create and add a node to Linked List 2
	if(((strcmp(animal_id, "") == 0)) && (min_reps == -1))
	{
		printk("\n--->>>No Input Parameter<<<---\n");
		printk("Thus, Both Linked List would be the Same");

		list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)
		{
			Add_Type_ID = kmalloc(sizeof(*Add_Type_ID), GFP_KERNEL);
			Allocated_Space_Bytes_2 += sizeof(*Add_Type_ID);
			strcpy(Add_Type_ID->Type_ID , Temp_Type_ID->Type_ID);
			Add_Type_ID->Reps = Temp_Type_ID->Reps;
			INIT_LIST_HEAD(&Add_Type_ID->list);
			list_add_tail(&(Add_Type_ID->list), &(Animals_Linked_List_2.list));
		}
	}
	else if((strcmp(animal_id, "") != 0)  && (min_reps == -1))
	{
		printk("\n--->>>One Input Parameter<<<---\n");
		printk("Filter-1 -> Animal Type ID: *%s*", animal_id);

		list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)
		{
			if(strcmp(animal_id, Temp_Type_ID->Type_ID) == 0 )
			{
				Add_Type_ID = kmalloc(sizeof(*Add_Type_ID), GFP_KERNEL);
				Allocated_Space_Bytes_2 += sizeof(*Add_Type_ID);
				strcpy(Add_Type_ID->Type_ID , Temp_Type_ID->Type_ID);
				Add_Type_ID->Reps = Temp_Type_ID->Reps;
				INIT_LIST_HEAD(&Add_Type_ID->list);
				list_add_tail(&(Add_Type_ID->list), &(Animals_Linked_List_2.list));
			}
		}
	}
	else if ((strcmp(animal_id, "") == 0) && (min_reps > -1))
	{
		printk("\n--->>>One Input Parameter<<<---\n");
		printk("Filter-2 -> Minimum Repetitions: *%d*", min_reps);

		list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)
		{
			if(Temp_Type_ID->Reps >= min_reps)
			{
				Add_Type_ID = kmalloc(sizeof(*Add_Type_ID), GFP_KERNEL);
				Allocated_Space_Bytes_2 += sizeof(*Add_Type_ID);
				strcpy(Add_Type_ID->Type_ID , Temp_Type_ID->Type_ID);
				Add_Type_ID->Reps = Temp_Type_ID->Reps;
				INIT_LIST_HEAD(&Add_Type_ID->list);
				list_add_tail(&(Add_Type_ID->list), &(Animals_Linked_List_2.list));
			}
		}
	}
	else
	{
		printk("\n--->>>Multiple Input Parameters<<<---\n");
		printk("Filter-1 -> Animal Type ID: *%s* Filter-2 -> Minimum Repetitions: *%d*", animal_id, min_reps);
		list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)
		{
			if((strcmp(animal_id, Temp_Type_ID->Type_ID) == 0) && (Temp_Type_ID->Reps >= min_reps))
			{
				Add_Type_ID = kmalloc(sizeof(*Add_Type_ID), GFP_KERNEL);
				Allocated_Space_Bytes_2 += sizeof(*Add_Type_ID);
				strcpy(Add_Type_ID->Type_ID , Temp_Type_ID->Type_ID);
				Add_Type_ID->Reps = Temp_Type_ID->Reps;
				INIT_LIST_HEAD(&Add_Type_ID->list);
				list_add_tail(&(Add_Type_ID->list), &(Animals_Linked_List_2.list));
			}
		}
	}
	
	printk("\nSpace (bytes) used by Linked List 2: %d", Allocated_Space_Bytes_2);

	// Linked List 2 Output using inbuilt looping function
	printk("\n--->>>Linked List 2 (Filtered)<<<---\n");
	list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_2.list, list)
	{
		printk("Animal Type ID: %s <-> Repetitionss: %d", Temp_Type_ID->Type_ID, Temp_Type_ID->Reps);
		node_counter_2 += 1;
	}
	printk("\nLinked List 2 has %d Nodes\n", node_counter_2);

	getnstimeofday(&time_exit); //Store the timestamp when module has finished processing (installation)

	time_required = timespec_sub(time_exit, time_entry); //Calculate time required for installation
	printk("\n--->>>Time time_required by this Module to Complete Processing and Installation is (in microseconds): %lu<<<---\n", (time_required.tv_nsec / 1000));
	return 0; 
}

// Uninstalling kernel module & freeing memory
static void __exit Custom_Geologist_Exit(void)
{
	getnstimeofday(&time_entry); //Store the timestamp when module has started processing (uninstalltion)
	
	printk("\nRemoving Custom Geologist Kernel Module\n\n");
	
	list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_1.list, list)	kfree(Temp_Type_ID);
	printk("Linked List 1 - All nodes Freed from memory, %d Bytes are Available to Kernel Now", Allocated_Space_Bytes_1); 

	list_for_each_entry(Temp_Type_ID, &Animals_Linked_List_2.list, list)	kfree(Temp_Type_ID);
	printk("Linked List 2 - All nodes Freed from memory, %d Bytes are Available to Kernel Now", Allocated_Space_Bytes_2);

	getnstimeofday (&time_exit); //Store the timestamp when module has finished processing (installation)
	
	time_required = timespec_sub(time_exit, time_entry); //Calculate time required for installation
	printk("\n--->>>Time time_required by this Module to Complete its Removal and Uninstallation is (in microseconds): %lu<<<---\n", (time_required.tv_nsec / 1000));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Poorn Mehta");
MODULE_DESCRIPTION("Custom Kernel Module for Geologists - Filters data as per the wish of User");

module_init(Custom_Geologist_Init);
module_exit(Custom_Geologist_Exit);
