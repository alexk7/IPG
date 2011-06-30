#include "Common.h"
#include "Grammar.h"

Def& AddDef(Defs& _defs, const std::string& _name, Expression& _e)
{
    std::pair<Defs::iterator, bool> result = _defs.insert(Def(_name, DefValue()));
    if (!result.second)
    {
        std::string errorMsg = "Duplicate definition: ";
        errorMsg += _name;
        throw std::runtime_error(errorMsg);
    }
    Def& newDef = *result.first;
    newDef.second.Swap(_e);
    return newDef;
}

void Print(std::ostream& _os, const Defs& _defs)
{
	Defs::const_iterator i, iEnd = _defs.end();
	for (i = _defs.begin(); i != iEnd; ++i)
	{
		_os << i->first << " <";
        if (i->second.isNode)
            _os << "=";
        else if (i->second.isMemoized)
            _os << "<";
        else
            _os << "-";
		_os << " " << i->second;
		_os << std::endl;
	}
}

void Print(std::ostream& _os, const Grammar& _grammar)
{
    Print(_os, _grammar.defs);
}
