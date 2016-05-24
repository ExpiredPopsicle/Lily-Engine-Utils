
# This function should merge lists and dictionaries together in a way
# that, I think, SCons should have a reasonable time understanding.
def merge(l1, l2):

    # Handle dictionaries. This is important for the BUILDERS
    # dictionary in the Lily Utils return stuff.
    if(type(l1) is dict):
        ret = l1.copy();
        if(type(l2) is dict):
            l2 = l2.copy();
            for k,v in l2.items():
                if(k in l1.keys()):
                    # Merge if this already exists.
                    ret[k] = merge(ret[k], l2[k]);
                else:
                    # Just set it if it doesn't already exist.
                    ret[k] = l2[k];
        else:
            # I'm not really sure what to do here.
            raise Exception("Can't merge dictionaries with lists.");
        return ret;

    # Concatenate lists.
    if(type(l1) is list):
        if(type(l2) is list):
            # Concatenate lists if they're both lists.
            return list(l1) + list(l2);
        # Otherwise just return a list with the item added.
        return list(l1) + [l2];

    # Default: Jut return a list of the two things.
    return [l1, l2];

