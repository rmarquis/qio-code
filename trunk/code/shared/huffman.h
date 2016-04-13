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

#include <shared/typedefs.h>

#define NYT HMAX					/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

#define HMAX 256 /* Maximum symbol */


class huffNode_c {
	friend class huffman_c;
	friend class huff_c;

	struct	huffNode_c *left, *right, *parent; /* tree structure */
	struct	huffNode_c *next, *prev; /* doubly-linked list */
	struct	huffNode_c **head; /* highest ranked node in block */
	int		weight;
	int		symbol;


	/* Get a symbol */
	friend int Huff_Receive(huffNode_c *node, int *ch, byte *fin);
	/* Swap these two nodes in the linked list (update ranks) */
	friend void swaplist(huffNode_c *node1, huffNode_c *node2);
	/* Do the increments */
	friend void increment(huff_c* huff, huffNode_c *node);

	/* Send the prefix code for this node */
	friend void send(huffNode_c *node, huffNode_c *child, byte *fout);

	// private ctor so no one will instance this class outside friend classes
	class huffNode_c() { }
};

class huff_c {
	friend class huffman_c;

	int			blocNode;
	int			blocPtrs;

	huffNode_c*		tree;
	huffNode_c*		lhead;
	huffNode_c*		ltail;
	huffNode_c*		loc[HMAX + 1];
	huffNode_c**	freelist;

	huffNode_c		nodeList[768];
	huffNode_c*		nodePtrs[768];

public:
	huffNode_c **get_ppnode();
	void free_ppnode(huffNode_c **ppnode);
	void swap(huffNode_c *node1, huffNode_c *node2);
	void transmit(int ch, byte *fout);
	void addRef(byte ch);
};

class huffman_c {
	huff_c		compressor;
	huff_c		decompressor;

	void initHuffman();
	void initForTable(int table[256]);
public:
	huffman_c(int table[256]) {
		initForTable(table);
	}

	void offsetReceive(int *ch, byte *fin, int *offset);
	void offsetTransmit(int ch, byte *fout, int *offset);


	// stateless in-place compression/decompression
	static void decompress(byte *data, int &cursize, int offset, int maxsize);
	static void compress(byte *data, int &cursize, int offset);
};

void	Huff_putBit(int bit, byte *fout, int *offset);
int		Huff_getBit(byte *fout, int *offset);
