/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2016 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Qio source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qio source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#include <shared/huffman.h>
#include <stdio.h>
#include <cstring>

static int			bloc = 0;

/* Add a bit to the output file (buffered) */
static void add_bit(char bit, byte *fout) {
	if ((bloc & 7) == 0) {
		fout[(bloc >> 3)] = 0;
	}
	fout[(bloc >> 3)] |= bit << (bloc & 7);
	bloc++;
}

/* Receive one bit from the input file (buffered) */
static int get_bit(byte *fin) {
	int t;
	t = (fin[(bloc >> 3)] >> (bloc & 7)) & 0x1;
	bloc++;
	return t;
}

/* Get a symbol */
int Huff_Receive(huffNode_c *node, int *ch, byte *fin) {
	while (node && node->symbol == INTERNAL_NODE) {
		if (get_bit(fin)) {
			node = node->right;
		}
		else {
			node = node->left;
		}
	}
	if (!node) {
		return 0;
		//		Com_Error(ERR_DROP, "Illegal tree!");
	}
	return (*ch = node->symbol);
}
/* Swap these two nodes in the linked list (update ranks) */
void swaplist(huffNode_c *node1, huffNode_c *node2) {
	huffNode_c *par1;

	par1 = node1->next;
	node1->next = node2->next;
	node2->next = par1;

	par1 = node1->prev;
	node1->prev = node2->prev;
	node2->prev = par1;

	if (node1->next == node1) {
		node1->next = node2;
	}
	if (node2->next == node2) {
		node2->next = node1;
	}
	if (node1->next) {
		node1->next->prev = node1;
	}
	if (node2->next) {
		node2->next->prev = node2;
	}
	if (node1->prev) {
		node1->prev->next = node1;
	}
	if (node2->prev) {
		node2->prev->next = node2;
	}
}

/* Do the increments */
void increment(huff_c* huff, huffNode_c *node) {
	huffNode_c *lnode;

	if (!node) {
		return;
	}

	if (node->next != NULL && node->next->weight == node->weight) {
		lnode = *node->head;
		if (lnode != node->parent) {
			huff->swap(lnode, node);
		}
		swaplist(lnode, node);
	}
	if (node->prev && node->prev->weight == node->weight) {
		*node->head = node->prev;
	}
	else {
		*node->head = NULL;
		huff->free_ppnode(node->head);
	}
	node->weight++;
	if (node->next && node->next->weight == node->weight) {
		node->head = node->next->head;
	}
	else {
		node->head = huff->get_ppnode();
		*node->head = node;
	}
	if (node->parent) {
		increment(huff, node->parent);
		if (node->prev == node->parent) {
			swaplist(node, node->parent);
			if (*node->head == node) {
				*node->head = node->parent;
			}
		}
	}
}

/* Send the prefix code for this node */
void send(huffNode_c *node, huffNode_c *child, byte *fout) {
	if (node->parent) {
		send(node->parent, node, fout);
	}
	if (child) {
		if (node->right == child) {
			add_bit(1, fout);
		}
		else {
			add_bit(0, fout);
		}
	}
}


void	Huff_putBit( int bit, byte *fout, int *offset) {
	bloc = *offset;
	if ((bloc&7) == 0) {
		fout[(bloc>>3)] = 0;
	}
	fout[(bloc>>3)] |= bit << (bloc&7);
	bloc++;
	*offset = bloc;
}

int		Huff_getBit( byte *fin, int *offset) {
	int t;
	bloc = *offset;
	t = (fin[(bloc>>3)] >> (bloc&7)) & 0x1;
	bloc++;
	*offset = bloc;
	return t;
}


huffNode_c **huff_c::get_ppnode() {
	huffNode_c **tppnode;
	if (!this->freelist) {
		return &(this->nodePtrs[this->blocPtrs++]);
	} else {
		tppnode = this->freelist;
		this->freelist = (huffNode_c **)*tppnode;
		return tppnode;
	}
}

void huff_c::free_ppnode(huffNode_c **ppnode) {
	*ppnode = (huffNode_c *)this->freelist;
	this->freelist = ppnode;
}

/* Swap the location of these two nodes in the tree */
void huff_c::swap (huffNode_c *node1, huffNode_c *node2) { 
	huffNode_c *par1, *par2;

	par1 = node1->parent;
	par2 = node2->parent;

	if (par1) {
		if (par1->left == node1) {
			par1->left = node2;
		} else {
	      par1->right = node2;
		}
	} else {
		this->tree = node2;
	}

	if (par2) {
		if (par2->left == node2) {
			par2->left = node1;
		} else {
			par2->right = node1;
		}
	} else {
		this->tree = node1;
	}
  
	node1->parent = par2;
	node2->parent = par1;
}

void huff_c::addRef(byte ch) {
	huffNode_c *tnode, *tnode2;
	if (this->loc[ch] == NULL) { /* if this is the first transmission of this node */
		tnode = &(this->nodeList[this->blocNode++]);
		tnode2 = &(this->nodeList[this->blocNode++]);

		tnode2->symbol = INTERNAL_NODE;
		tnode2->weight = 1;
		tnode2->next = this->lhead->next;
		if (this->lhead->next) {
			this->lhead->next->prev = tnode2;
			if (this->lhead->next->weight == 1) {
				tnode2->head = this->lhead->next->head;
			} else {
				tnode2->head = get_ppnode();
				*tnode2->head = tnode2;
			}
		} else {
			tnode2->head = get_ppnode();
			*tnode2->head = tnode2;
		}
		this->lhead->next = tnode2;
		tnode2->prev = this->lhead;
 
		tnode->symbol = ch;
		tnode->weight = 1;
		tnode->next = this->lhead->next;
		if (this->lhead->next) {
			this->lhead->next->prev = tnode;
			if (this->lhead->next->weight == 1) {
				tnode->head = this->lhead->next->head;
			} else {
				/* this should never happen */
				tnode->head = get_ppnode();
				*tnode->head = tnode2;
		    }
		} else {
			/* this should never happen */
			tnode->head = get_ppnode();
			*tnode->head = tnode;
		}
		this->lhead->next = tnode;
		tnode->prev = this->lhead;
		tnode->left = tnode->right = NULL;
 
		if (this->lhead->parent) {
			if (this->lhead->parent->left == this->lhead) { /* lhead is guaranteed to by the NYT */
				this->lhead->parent->left = tnode2;
			} else {
				this->lhead->parent->right = tnode2;
			}
		} else {
			this->tree = tnode2; 
		}
 
		tnode2->right = tnode;
		tnode2->left = this->lhead;
 
		tnode2->parent = this->lhead->parent;
		this->lhead->parent = tnode->parent = tnode2;
     
		this->loc[ch] = tnode;
 
		increment(this, tnode2->parent);
	} else {
		increment(this, this->loc[ch]);
	}
}


/* Get a symbol */
void huffman_c::offsetReceive (int *ch, byte *fin, int *offset) {
	huffNode_c *node = this->decompressor.tree;
	bloc = *offset;
	while (node && node->symbol == INTERNAL_NODE) {
		if (get_bit(fin)) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	if (!node) {
		*ch = 0;
		return;
//		Com_Error(ERR_DROP, "Illegal tree!");
	}
	*ch = node->symbol;
	*offset = bloc;
}

/* Send a symbol */
void huff_c::transmit (int ch, byte *fout) {
	int i;
	if (this->loc[ch] == NULL) { 
		/* huffNode_c hasn't been transmitted, send a NYT, then the symbol */
		transmit(NYT, fout);
		for (i = 7; i >= 0; i--) {
			add_bit((char)((ch >> i) & 0x1), fout);
		}
	} else {
		send(this->loc[ch], NULL, fout);
	}
}

void huffman_c::offsetTransmit (int ch, byte *fout, int *offset) {
	huff_c *huff = &this->compressor;
	bloc = *offset;
	send(huff->loc[ch], NULL, fout);
	*offset = bloc;
}

void huffman_c::decompress(byte *data, int &cursize, int offset, int maxsize) {
	int			ch, cch, i, j, size;
	byte		seq[65536];
	byte*		buffer;
	huff_c		huff;

	size = cursize - offset;
	buffer = data + offset;

	if ( size <= 0 ) {
		return;
	}

	memset(&huff, 0, sizeof(huff_c));
	// Initialize the tree & list with the NYT node 
	huff.tree = huff.lhead = huff.ltail = huff.loc[NYT] = &(huff.nodeList[huff.blocNode++]);
	huff.tree->symbol = NYT;
	huff.tree->weight = 0;
	huff.lhead->next = huff.lhead->prev = NULL;
	huff.tree->parent = huff.tree->left = huff.tree->right = NULL;

	cch = buffer[0]*256 + buffer[1];
	// don't overflow with bad messages
	if ( cch > maxsize - offset ) {
		cch = maxsize - offset;
	}
	bloc = 16;

	for ( j = 0; j < cch; j++ ) {
		ch = 0;
		// don't overflow reading from the messages
		// FIXME: would it be better to have an overflow check in get_bit ?
		if ( (bloc >> 3) > size ) {
			seq[j] = 0;
			break;
		}
		Huff_Receive(huff.tree, &ch, buffer);				/* Get a character */
		if ( ch == NYT ) {								/* We got a NYT, get the symbol associated with it */
			ch = 0;
			for ( i = 0; i < 8; i++ ) {
				ch = (ch<<1) + get_bit(buffer);
			}
		}
    
		seq[j] = ch;									/* Write symbol */

		huff.addRef((byte)ch);								/* Increment node */
	}
	cursize = cch + offset;
	memcpy(data + offset, seq, cch);
}

void huffman_c::compress(byte *data, int &cursize, int offset) {
	int			i, ch, size;
	byte		seq[65536];
	byte*		buffer;
	huff_c		huff;

	size = cursize - offset;
	buffer = data+ + offset;

	if (size<=0) {
		return;
	}

	memset(&huff, 0, sizeof(huff_c));
	// Add the NYT (not yet transmitted) node into the tree/list */
	huff.tree = huff.lhead = huff.loc[NYT] =  &(huff.nodeList[huff.blocNode++]);
	huff.tree->symbol = NYT;
	huff.tree->weight = 0;
	huff.lhead->next = huff.lhead->prev = NULL;
	huff.tree->parent = huff.tree->left = huff.tree->right = NULL;
	huff.loc[NYT] = huff.tree;

	seq[0] = (size>>8);
	seq[1] = size&0xff;

	bloc = 16;

	for (i=0; i<size; i++ ) {
		ch = buffer[i];
		huff.transmit(ch, seq);						/* Transmit symbol */
		huff.addRef((byte)ch);								/* Do update */
	}

	bloc += 8;												// next byte

	cursize = (bloc>>3) + offset;
	memcpy(data+offset, seq, (bloc>>3));
}

void huffman_c::initHuffman() {
	memset(&this->compressor, 0, sizeof(huff_c));
	memset(&this->decompressor, 0, sizeof(huff_c));

	// Initialize the tree & list with the NYT node 
	this->decompressor.tree = this->decompressor.lhead = this->decompressor.ltail = this->decompressor.loc[NYT] = &(this->decompressor.nodeList[this->decompressor.blocNode++]);
	this->decompressor.tree->symbol = NYT;
	this->decompressor.tree->weight = 0;
	this->decompressor.lhead->next = this->decompressor.lhead->prev = NULL;
	this->decompressor.tree->parent = this->decompressor.tree->left = this->decompressor.tree->right = NULL;

	// Add the NYT (not yet transmitted) node into the tree/list */
	this->compressor.tree = this->compressor.lhead = this->compressor.loc[NYT] =  &(this->compressor.nodeList[this->compressor.blocNode++]);
	this->compressor.tree->symbol = NYT;
	this->compressor.tree->weight = 0;
	this->compressor.lhead->next = this->compressor.lhead->prev = NULL;
	this->compressor.tree->parent = this->compressor.tree->left = this->compressor.tree->right = NULL;
	this->compressor.loc[NYT] = this->compressor.tree;
}

void huffman_c::initForTable(int table[256]) {
	initHuffman();
	for (int i = 0; i<256; i++) {
		for (int j = 0; j<table[i]; j++) {
			compressor.addRef((byte)i);			// Do update
			decompressor.addRef((byte)i);			// Do update
		}
	}
}
