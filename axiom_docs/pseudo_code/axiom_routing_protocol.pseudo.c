/*!
 * axiom_routing_protocol.pseudo.c
 *
 * \version     v0.9
 * \date        2016-03-08
 *
 * This file contains the pseudo code of AXIOM Routing Protocol
 *
 */

A. Nodo Master data la Topologia della rete calcola NUM_NODES tabelle di routing (una per ogni nodo)

   utilizzando l'alogoritmo di simil-Dijkstra:
 
   1. Initialization:
	N = {A}       
	for all nodes v:
	    if v adjacent to A
	 	else d(v) = 1

    2. loop
	find node w not in set N such that d(w) is smallest
	add w into N
		update d(v) for all v not in N:
		d(v) = min{d(v); d(w) + c(w; v)}
       until all nodes are in N

B. Algoritmo di distribuzione della routing table:
    
    1. Il Master setta la propria routing table
   
    2. Il Master invia la tabella di routing ai suoi vicini utilzzando i RAW messages

    3. 
	while (NUM_NODES tabelle non sono state inviate)
	    
            Ciascun nodo che riceve la tabella di routing esegue la axiom_set_routing()

	    Il nodo Master invia le tabelle dei vicini
	end while

Possibile problema: Quando vengono inviate le tabelle ai vicini di n-esimo livello, 
			routing table dei vicini dell' (n-1)-esimo livello devono 
			essere state già effettivamente impostate dall'hardware.
Per essere sicuri di ciò potremmo impostare un ritardo tra l'invio delle tabelle di
livelli adiacenti.   
