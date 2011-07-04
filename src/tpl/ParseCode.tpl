{{#isMemoized}}PTNode* p1 = GetEnd(p0, PTNodeType_{{name}});
if (p1)
	return p1;
{{/isMemoized}}
{{>expression}}
{{#isMemoized}}SetEnd(p0, PTNodeType_{{name}}, p{{resultIndex}});
{{/isMemoized}}
return p1;
