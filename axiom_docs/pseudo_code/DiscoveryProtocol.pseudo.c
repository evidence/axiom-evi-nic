/********************************************************/
/************ Discovery Algorithm pseudo-code ***********/
/* The Topology matrix of the master node, at the end of*/
/* the master_node_code() function execution, will      */
/* contain the topology of the network .                */
/* The Topology matrix of the other nodes, at the end of*/
/* the slave_node_code() function execution, will       */
/* contain a partial topology of the network            */
/********************************************************/


/* axiom_send_raw_neighbour: a parameter with zero value
   indicates it is not significant in the corresponding 
   message type 
*/




#define NUMBER_OF_NODE          256u
#define NUMBER_OF_INTERFACE     4u

/*
 * @brief Discover algorithm pseudo-code.
 * @param topology: Matrix memorizing the topolgy of the network
 * @param next_id:  Actual Node Identification value
 */
/* discover algorithm pseudo-code */
void discover_phase(axiom_node_id_t *next_id, axiom_node_id_t topology[][NUMBER_OF_INTERFACES])
{
    < Read My Node Id (my_node_id) >

    < Get the number of the connected interfaces > 

    < For Each 'i' active interface >
	
	< Say over interface 'i':I am node my_node_id give me your node id >
        axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_REQ_ID, my_node_id, 0, i, 0, 0);

	< Wait for the neighbourg response >
	axiom_recv_raw_neighbour (&msg_type, &src_node_id, &dst_node_id, &src_interface, &dst_interface, &data);

	< The neighbour has an ID >
	if (msg_type == AXIOM_DSCV_TYPE_RSP_ID)
	    
	    < Immediately update my routing table: the neighbour node is connceted to my 'i' interface >
            axiom_set_routing(neighbour, i);

	    < Update the topology data structure:  my 'i' interface is connected  the neighbour node
                			           The neighbour node is connected to me on its  'src_interface'>  


	< The neighbour has no ID >
	if (msg_type == AXIOM_DSCV_TYPE_RSP_NOID)	
	    
	    < Say over interface 'i': you are node next_id  >
            axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_SETID, my_node_id, 0, i, 0, *next_id);

	    < Immediately update my routing table: next_id node is connceted to my 'i' interface >
            axiom_set_routing(*next_id, i);	

	    < Update the topology data structure:  my 'i' interface is connected  the new next_id node
                			           next_id node is connected to me on its  'src_interface'>  

	    < Say over interface 'i': start discovery protocol, nextid >
	    < @Note: this message could be omitted; the next_id node could start its discovery protocol
		     after receving the AXIOM_DSCV_TYPE_SETID type message >   
            axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_START, my_node_id, *next_id, i, 0, 0);  

	    
	    < wait for the messages with the next_id topology data structure >
            while (msg_type != AXIOM_DSCV_TYPE_END_TOPOLOGY)
		
	        < Receive a  message with:
			- 'i' interface connected Neighbour node topology 
			- the new updated nextid
                   	- A Neighbour ID request on e non-'i' interface
		>
                axiom_recv_raw_neighbour (&msg_type, &src_node_id, &dst_node_id, &src_interface, &dst_interface, &data);

	    	< request for my id from a node which is executing its discovery algorithm >
                if (msg_type == AXIOM_DSCV_TYPE_REQ_ID)
                        < Immediately update my routing table: 
			  'src_node_id' node is connceted to my 'dst_interface' interface
			>
                        axiom_set_routing(src_node_id, dst_interface);

                        < Reply: I am node 'my_node_id', I am on interface 'src_interface' >
                        axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_RSP_ID, my_node_id, dst_node_id, src_interface, dst_interface, 0);

		< topology info from the 'i' interface connected Neighbour>
                else if (msg_type == AXIOM_DSCV_TYPE_TOPOLOGY)
                   < Update the topology data structure with the received information >

		< End of neighbour discovery protocol, it sends me the new updated nextid >
	        else if (msg_type == AXIOM_DSCV_TYPE_END_TOPOLOGY)
		    *next_id = data;

	    < end While >
	
    < End For Each 'i' active interface >
}
/*
 * @brief Master node Discovery Algorithm pseudo-code.
 * @param topology: Matrix memorizing the topolgy of the network
 * (a row for each node, a column for each interface)
 * topology[id_node1][id_iface1] = (id_node2, id_iface) means that
   id_node1 node, on its id_iface_1 is connected with
   id_node12node, on its id_iface_2
 */
void master_node_code(axiom_node_id_t topology[][NUMBER_OF_INTERFACES])
{
    axiom_node_id_t next_id = 0;

    /*  Initializes the Topology matrix: no node is connected */
    init_topology_structure(topology);

    /* Start the discovery phase*/
    discover_phase(&next_id ,topology);
}

/*
 * @brief Slave node Discovery Algorithm pseudo-code.
 * @param topology: Matrix memorizing the topolgy of the network
 * (a row for each node, a column for each interface)
 * topology[id_node1][id_iface1] = (id_node2, id_iface) means that
   id_node1 node, on its id_iface_1 is connected with
   id_node12node, on its id_iface_2
 */
void slave_node_code(axiom_node_id_t topology[][NUMBER_OF_INTERFACES])
{
    < Read My Node Id (my_node_id) >

    < Wait for the neighbour AXIOM_DSCV_TYPE_REQ_ID type message >
    while (msg_type != AXIOM_DSCV_TYPE_REQ_ID)
       
        axiom_recv_raw_neighbour (&msg_type, &src_node_id, &dst_node_id, &src_interface, &dst_interface, &data);
	
        < Immediately update my routing table: the neighbour node is connceted to my interface >
    	axiom_set_routing(neighbour_id, my_interface);

	< If I already have a node Id >
	if (my_node_id == 0) 
	    
	    < Reply 'I am node 'my_node_id', I'm on interface 'src_interface' >
            axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_RSP_ID, my_node_id, dst_node_id, src_interface, dst_interface, 0);

	< If I have not an Id >
	else

	    < Wait for the neighbour AXIOM_DSCV_TYPE_START type message >
            while (msg_type != AXIOM_DSCV_TYPE_START)
                axiom_recv_raw_neighbour(&msg_type, &src_node_id, &dst_node_id, &src_interface, &dst_interface, &data);

	    next_id = dst_node_id; < that is my id, previously yet recevied into data field>

	    < Start the dicovery algorithm >
            discover_phase(&next_id, topology);

	    < Send topology (Node1, if1, Node2, If2) list to the node which sent me the start message >
	    axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_TOPOLOGY, Node1, if1, dst_interface, Node2, if2);

	    < Says: <<Finished sending the topology structure, I send back actual next_id>> 
            axiom_send_raw_neighbour(AXIOM_DSCV_TYPE_END_TOPOLOGY,0, 0, dst_interface, 0, next_id);
}
