#ifndef RANKINGTREE_H
#define RANKINGTREE_H

/* Warning.

   Deletion / insertion / key changes are not well protected so the user can do
    too much so test thoroughly after changing any code involving RankingTrees and their node.

   changeKey() can be optimized and then would not necessit returning a new node. (removal/reinsertion)
*/

/* Red - Black tree modified so that ranking is efficiently managed. Everything is in O(log n). */
template <class T>
class RankingTree
{
public:
    enum Color
    {
        Red = 0,
        Black = 1
    };

    struct Node
    {
        void changeColor(Color c)
        {
            black = bool(c);
        }
        Color color() const
        {
            return Color(black);
        }
        Node *grandParent() const
        {
            if (parent)
                return parent->parent;
            else
                return NULL;
        }
        Node *uncle() const
        {
            Node *g = grandParent();
            if (g) {
                return g->right == parent ? g->left : g->right;
            } else
                return NULL;
        }
        Node *sibling() const
        {
            if (this == parent->left)
                return parent->right;
            else
                return parent->left;
        }
        void decreaseCountUp()
        {
            count -= 1;
            if (parent)
            {
                parent->decreaseCountUp();
            }
        }
        void increaseCountUp()
        {
            count += 1;
            if (parent)
                parent->increaseCountUp();
        }
        int ranking()
        {
            return (right ? right->count : 0) + countUp();
        }

        int countUp()
        {
            if (parent == NULL)
                return 1;
            if (this == parent->right)
                return parent->countUp();
            else //(this == parent->left)
                return parent->ranking() + 1;
        }


        const Node *utmostLeft() const
        {
            return left == NULL ? this : left->utmostLeft();
        }
        const Node *utmostRight() const
        {
            return right == NULL ? this : right->utmostLeft();
        }
        Node *utmostLeft()
        {
            return left == NULL ? this : left->utmostLeft();
        }
        Node *utmostRight()
        {
            return right == NULL ? this : right->utmostRight();
        }
        Node *prev() const
        {
            if (left)
                return left->utmostRight();
            if (!parent) {
                /* No left, no parent, its the end! */
                return NULL;
            }
            if (this == parent->left) {
                return parent->goLeftUp();
            } else /* this == parent->righ */
                return parent;

        }
        Node *next() const
        {
            if (right)
                return right->utmostLeft();
            if (!parent) {
                /* No right, no parent, its the end! */
                return NULL;
            }
            if (this == parent->right) {
                return parent->goRightUp();
            } else /* this == parent->left */
                return parent;

        }
        Node *goLeftUp()
        {
            if (!parent)
                return NULL;
            if (this == parent->right)
                return parent;
            return parent->goLeftUp();
        }
        Node *goRightUp()
        {
            if (!parent)
                return NULL;
            if (this == parent->left)
                return parent;
            return parent->goRightUp();
        }
        void recursiveDelete()
        {
            if (left)
                left->recursiveDelete();
            if (right)
                right->recursiveDelete();
            delete this;
        }

        Node(int key, Color c = Black, Node *parent = NULL, T data = T()) : parent(parent), left(NULL), right(NULL), key(key), black(c), count(1),
                                                                            data(data)
        {

        }


        Node *parent;
        Node *left, *right;
        int key;
        bool black;
        /* The number of nodes under it */
        int count;
        T data;
    };


    RankingTree() : root(NULL) {

    }
    ~RankingTree() {
        if (root) {
            root->recursiveDelete();
            root = NULL;
        }
    }

    int count() const
    {
        return root ? root->count : 0;
    }

    Node* insert(int key, T data);

    /* Could manage without deleting the node, with proper insertion / removal */
    Node *changeKey(Node *n, int key)
    {
        if (key == n->key)
            return n;

        Node *ret = insert(key, n->data);
        deleteNode(n);

        return ret;
    }

    /*
     * The Node was already inserted, it's red. For all we know,
     * it might be the root.
     */
    void insertCase1(Node *p)
    {
        if (root == p)
        {
            p->changeColor(Black);
        } else {
            insertCase2(p);
        }
    }

    /*
     * The Node was already inserted, it's red, and its not the root.
     * We have to fix any violation
     */
    void insertCase2(Node *p)
    {
        /* If the parent is black, its child being red poses no problem */
        if (p->parent->color() != Black)
            insertCase3(p);
    }

    /*
     * The Node was already inserted, it's red, and it has a red parent (problem!)
     * We have to fix this!
     */
    void insertCase3(Node *p)
    {
        /* Lets check if the uncle is red. If the uncle is, then we change both the
           parent and the uncle to black. The grandparent was black, as it had red
           children. So we change its color to red (to have the same number of blacks
           in every path. By changing it to red, we may have violated the rule that
           the root is black or that every red node has black leaves.  */
        Node *u = p->uncle();

        if (u && u->color() == Red)
        {
            u->changeColor(Black);
            p->parent->changeColor(Black);
            Node *g = p->grandParent();
            g->changeColor(Red);
            insertCase1(g);
        } else {
            insertCase4(p);
        }
    }

    /*
     * We have a situation. p is Red and its parent is red and its uncle is black.
     * We'll need two rotations to restore the tree
     */
    void insertCase4(Node *n)
    {
        Node *g = n->grandParent();

        if ((n == n->parent->right) && (n->parent == g->left)) {
                rotateLeft(n->parent);
                n = n->left;
        } else if ((n == n->parent->left) && (n->parent == g->right)) {
                rotateRight(n->parent);
                n = n->right;
        }

        if (n)
            insertCase5(n);
    }

    /*
     * In continuation of insertCase4
     */
    void insertCase5(Node *n)
    {
        Node *g = n->grandParent();

        n->parent->changeColor(Black);
        g->changeColor(Red);
        if ((n == n->parent->left) && (n->parent == g->left)) {
             rotateRight(g);
        } else { /* (n == n->parent->right) and (n->parent == g->right) */
             rotateLeft(g);
        }
    }

    void rotateRight(Node *p);
    void rotateLeft(Node *p);

    void deleteNode(Node *n);

    /*
     * Precondition: n has at most one non-null child.
     */
    void deleteOneChild(Node *n);

    /*
     * We must sort out things as in all paths including that node
     * there'll be one less black node
     */
    void deleteCase1(Node *n)
    {
        /* If we are the root, then there's nothing to worry about */
        if (n->parent == NULL) {

        } else {
            deleteCase2(n);
        }

    }

    /*
     * A black node that is not the root will have a black node less
     * in all its paths.
     */
    void deleteCase2(Node *n)
    {
        Node *s = n->sibling();
        if (s->color() == Red)
        {
            /* We must perform a rotation to rebalance the tree.
               Its n's side that loses a black in all its paths, so
               it's why we give more nodes to that side. */
            n->parent->changeColor(Red);
            s->changeColor(Black);
            if (n == n->parent->left) {
                rotateLeft(n->parent);
            } else {
                rotateRight(n->parent);
            }
            deleteCase4(n);
        } else {
            deleteCase3(n);
        }
    }

    /*
     * The node is black, will have a black node less in all its paths and
     * its sibling is black.
     *
     */
    void deleteCase3(Node *n)
    {
        /* If the parent is also black, and the sibling's children are black / NULL too,
         * then we change the sibling to red. The problem is fixed for the tree under
         * the parent but now the parent will have 1 less black node for path going through it
         * so we go back to the start from the parent this time. */
        Node *s = n->sibling();
        if (n->parent->color() == Black && (!s->left || s->left->color() == Black) && (!s->right || s->right->color() == Black))
        {
            s->changeColor(Red);
            deleteCase1(n->parent);
        } else {
            deleteCase4(n);
        }
    }

    /*
     * The node is black, will have a black node less in all its paths and
     * its sibling is black.
     *
     * But either its parent is red or/and the sibling has at least 1 red child.
     */
    void deleteCase4(Node *n)
    {
        Node *s = n->sibling();

        /* If the parent is red, we switch colors between the parent and the sibling (if possible) and that's done. */
        if (n->parent->color() == Red && (!s->left || s->left->color() == Black) && (!s->right || s->right->color() == Black)) {
            n->parent->changeColor(Black);
            s->changeColor(Red);
        } else {
            deleteCase5(n);
        }

    }

    /*
     * The node is black, will have a black node less in all its paths and
     * its sibling is black.
     *
     * But the sibling has at least 1 red child and the parent is unknown color.
     */
    void deleteCase5(Node *n)
    {
        Node *s = n->sibling();
        /* We will have to perform a rotation so we relocate the red child if its not on
           the right side of the sibling.

           We force red to be on the right of the right or left of the left of the parent. */
        if ((n == n->parent->left) && (!s->right || s->right->color() == Black)) {
            /* Red child is on left of s */
            s->changeColor(Red);
            s->left->changeColor(Black);
            rotateRight(s);
        } else if ((n == n->parent->right) && (!s->left || s->left->color() == Black)) {
            /* Red child is on right of s */
            s->changeColor(Red);
            s->right->changeColor(Black);
            rotateLeft(s);
        }
        /* And now the rotation... */
        deleteCase6(n);
    }

    /*
     * The node is black, will have a black node less in all its paths and
     * its sibling is black.
     *
     * But the sibling has at least 1 red child and the parent is unknown color.
     *
     * If the sibling is left of the parent then its left is a red child, else
     * if the sibling is right of the parent then its right is a red child
     */
    void deleteCase6(Node *n)
    {
        Node *s = n->sibling();

        s->changeColor(n->parent->color());
        n->parent->changeColor(Black);

        if (n == n->parent->left) {
            s->right->changeColor(Black);
            rotateLeft(n->parent);
        } else {
            s->left->changeColor(Black);
            rotateRight(n->parent);
        }

        /* Now n is ready for deletion. Won't cause no damage! */
    }



    struct iterator {
        mutable Node *p;

        iterator(Node *p = NULL) : p(p)
        {

        }

        const iterator& operator ++ () const {
            p = p ? p->next() : p;
            return *this;
        }

        iterator& operator ++ () {
            p = p ? p->next() : p;
            return *this;
        }

        const iterator& operator -- () const {
            p = p ? p->prev() : p;
            return *this;
        }

        iterator& operator -- () {
            p = p ? p->prev() : p;
            return *this;
        }

        Node& operator *() const
        {
            return *p;
        }

        Node * node() const
        {
            return p;
        }

        Node * operator ->() const
        {
            return &(operator *());
        }

        bool operator ==(const iterator &other)
        {
            return p == other.p;
        }

        bool operator !=(const iterator &other)
        {
            return p != other.p;
        }
    };

    typedef const iterator const_iterator;

    /* This is something like O(log(n)*log(log(n))).
       Could be O(log(n)) if you aren't lazy */
    iterator getByRanking(int ranking)
    {
        if (root == NULL)
            return iterator();

        if (ranking > count()) {
            return root->utmostLeft();
        }

        if (ranking <= 1)
            return root->utmostRight();

        Node *ptr = root;

        int r;
        while ((r = ptr->ranking()) != ranking)
        {
            if (r < ranking)
                ptr = ptr->left;
            else
                ptr = ptr->right;
        }

        return ptr;
    }

    const_iterator getByRanking(int ranking) const
    {
        if (root == NULL)
            return iterator();

        if (ranking > count()) {
            return NULL;
        }

        if (ranking <= 1)
            return root->utmostRight();

        Node *ptr = root;

        int r;
        while ((r = ptr->ranking()) != ranking)
        {
            if (r < ranking) {
                if (ptr->left == NULL) {
                    //qDebug() << "Critical: ranking " << ranking << " left not found, instead " << r;
                    return ptr;
                }
                ptr = ptr->left;
            }
            else {
                if (ptr->right == NULL) {
                    //qDebug() << "Critical: ranking " << ranking << " right not found, instead " << r;
                    return ptr;
                }
                ptr = ptr->right;
            }
        }

        return ptr;
    }


    const_iterator begin() const {
        return root ? iterator(root->utmostLeft()) : iterator(root);
    }

    const_iterator end() const {
        return iterator(NULL);
    }


    RankingTree(const RankingTree &other) throw (const char*) {
        if (other.root) {
            throw "Error: Copying ranking tree that's not empty";
        }
        root = NULL;
    }
    Node *root;
};

template<class T>
void RankingTree<T>::deleteNode(Node *n)
{
    if (n->left && n->right) {
        /* Two children, we find the one child successor and swap and delete the node */
        /* The reason we swap them is because we MUST KEEP POINTERS VALID! */
        /* And all the tests are to treat special case like when a parent is swapped with
           its child */
        Node * next = n->next();

        /* Swap node! */
        Node n_(n->key);
        n_.left = n->left;
        n_.right = n->right;
        n_.parent = n->parent;
        n_.count = n->count;
        n_.changeColor(n->color());

        n->left = next->left;
        n->right = next->right;
        n->parent = next->parent == n ? next : next->parent;
        n->changeColor(next->color());

        next->parent = n_.parent;
        next->right = n_.right == next ? n : n_.right;
        next->left = n_.left == next ? n : n_.left;
        next->changeColor(n_.color());

        n->count = next->count;
        next->count = n_.count;

        if (n->left)
            n->left->parent = n;
        if (n->right)
            n->right->parent = n;
        if (n->parent && n->parent!= next)
        {
            if (next == n->parent->left) {
                n->parent->left = n;
            } else {
                n->parent->right = n;
            }
        }
        if (next->right)
            next->right->parent = next;
        if (next->left)
            next->left->parent = next;
        if (next->parent) {
            if (n == next->parent->left) {
                next->parent->left = next;
            } else {
                next->parent->right = next;
            }
        }
        if (root == n) {
            root = next;
        }
    }
    deleteOneChild(n);
}

template <class T>
typename RankingTree<T>::Node* RankingTree<T>::insert(int key, T data)
{
    /* If there's no root, it's trivial... */
    if (root == NULL)
    {
        root = new Node(key, Black, NULL, data);
        return root;
    }
    /* Now, we have to find the correponding leaf, insert it,
       make the necessary color change / rotation */
    /* Finding the leaf */
    Node *ptr = root;
    while (1)
    {
        if (key > ptr->key) {
            /* We have to look to the right side ... */
            if (ptr->right) {
                ptr = ptr->right;
            } else {
                /* The right leaf is empty, so we are supposed to insert the new node here */
                ptr = ptr->right = new Node(key, Red, ptr, data);
                break;
            }
        } else {
            /* We have to look to the left side ... */
            if (ptr->left) {
                ptr = ptr->left;
            } else {
                /* The left leaf is empty, so we are supposed to insert the new node here */
                ptr = ptr->left = new Node(key, Red, ptr, data);
                break;
            }
        }
    }
    /* We increase the count of every parent as we keep track of it */
    ptr->parent->increaseCountUp();
    /* Now, we've inserted a new leaf, red, with a parent. */
    /* We need to fix the tree in case the rules are violated */
    insertCase2(ptr);
    return ptr;
}

template<class T>
void RankingTree<T>::rotateRight(Node *p)
{
    p->left->parent = p->parent;
    if (p->parent) {
        if (p->parent->left == p)
            p->parent->left = p->left;
        else
            p->parent->right = p->left;
    }
    p->parent = p->left;

    p->left = p->parent->right;
    if (p->left)
        p->left->parent = p;

    p->parent->right = p;

    if (root == p)
        root = p->parent;

    /* Updating the count for the rank algorithm */
    p->count = (p->right ? p->right->count : 0) + (p->left ? p->left->count : 0) + 1;
    p->parent->count =  (p->parent->left ? p->parent->left->count : 0) + p->count + 1;
}

template<class T>
void RankingTree<T>::rotateLeft(Node *p)
{
    p->right->parent = p->parent;
    if (p->parent) {
        if (p->parent->right == p)
            p->parent->right = p->right;
        else
            p->parent->left = p->right;
    }
    p->parent = p->right;

    p->right = p->parent->left;
    if (p->right)
        p->right->parent = p;

    p->parent->left = p;

    if (root == p)
        root = p->parent;

    /* Updating the count for the rank algorithm */
    p->count = (p->right ? p->right->count : 0) + (p->left ? p->left->count : 0) + 1;
    p->parent->count =  (p->parent->right ? p->parent->right->count : 0) + p->count + 1;
}

/*
 * Precondition: n has at most one non-null child.
 */
template<class T>
void RankingTree<T>::deleteOneChild(Node *n)
{
    Node *child = n->right ? n->right : n->left;

    /* If theres no child, its either a red node (easy) or black node (hard)*/
    if (!child) {
        if (n->color() == Red) {
            /* No worry */
        } else {
            /* We need to rebalance the tree as the parent has 1 black node
             * less coming from us */
            deleteCase1(n);
        }
    } else {
        /* Means 1 child, means we are black and the child is red */
        child->changeColor(Black);
        child->parent = n->parent;
    }
    if (n == root) {
        root = child;
    } else {
        if (n->parent->left == n) {
            n->parent->left = child;
        } else {
            n->parent->right = child;
        }
    }

    n->decreaseCountUp();
    delete n;
}

#endif // RANKINGTREE_H
#ifndef RANKINGTREE_H
#define RANKINGTREE_H

#endif // RANKINGTREE_H
