
#pragma once

// only supports unsigned int keys with less comparison for now

template<typename ValType>
	class flmap	
	{	

	public:

		struct Node;

		typedef Node* NodePtr;

		struct Node
		{
			NodePtr left;
			NodePtr parent;
			NodePtr right;
			unsigned int key;
			ValType data;
		};

		class iterator
		{
		public:
			iterator(NodePtr currentNode, flmap<ValType>* classPtr)
			{
				_currentNode = currentNode;
				_classObj = classPtr;
			}

			iterator()
			{
				_currentNode = 0;
				_classObj = 0;
			}

			void Inc()
			{
				if(_classObj->IsNil(_currentNode) == true)
					;
				else if(_classObj->IsNil(_currentNode->right) == false)
					_currentNode = _classObj->Min(_currentNode->right);
				else
				{ // climb looking for right subtree
					NodePtr node;
					while(_classObj->IsNil(node = _currentNode->parent) == false && _currentNode == node->right)
						_currentNode = node; // ==> parent while right subtree
					_currentNode = node; // ==> parent (head if end())
				}

				// set to end node if we are at nil (so we can compare against end-iterator)
				if(_classObj->IsNil(_currentNode))
					_currentNode = _classObj->end()._currentNode;
			}

			iterator& operator++() {
				Inc();
				return *this;
			}
			iterator operator++(int) {
				iterator tmp(*this); // copy
				operator++(); // pre-increment
				return tmp;   // return old value
			}

			bool operator==(const iterator& _Right) const
			{	// test for iterator equality
				return (_currentNode == _Right._currentNode);
				}

			bool operator!=(const iterator& _Right) const
				{	// test for iterator inequality
				return (!(*this == _Right));
				}
			
			unsigned int * key()
			{
				return &_currentNode->key;
			}

			ValType* value()
			{
				return &_currentNode->data;
			}

		private:
			NodePtr _currentNode;
			flmap<ValType>* _classObj;
		};

	public:

		unsigned int size() { return _size;};

		iterator begin()
		{
			return iterator(_headNode->left, this);
		}

		iterator end()
		{
			return iterator(_endNode, this);
		}

		iterator find(unsigned int key)
		{
			NodePtr searchNode = _headNode->parent; // parent of headnode is first legit (upmost) node

			while(IsNil(searchNode) == false)
			{
				if(searchNode->key == key)
					break;

				if(key < searchNode->key)
					searchNode = searchNode->left;
				else
					searchNode = searchNode->right;
			}

			return iterator(searchNode, this);
		}

	protected:
		NodePtr Min(NodePtr node)
		{
			// go to leftmost child
			while(IsNil(node->left) == false)
				node = node->left;

			return node;
		}

		bool IsNil(NodePtr node)
		{
			return (node == _endNode || node == _headNode);
		}

	private:
		void* dunno;
		NodePtr _headNode; // headnode stores min/max in left/right and upmost node in parent
		NodePtr _endNode;
		void* dunno2;
		unsigned int _size;	
	};
