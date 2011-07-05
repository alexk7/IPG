{{#choice}}
{{/choice}}

{{#zeroOrMore}}
{{#tmp}}{{>var}}p{{resultIndex}} = p{{firstIndex}}{{/tmp}}
{{/zeroOrMore}}

{{#nonTerminal_Visit}}
{{>var}}p{{resultIndex}} = Visit(p{{firstIndex}}, PTNodeType_{{name}}, v);
{{/nonTerminal_Visit}}

{{#nonTerminal_GetEnd}}
{{>var}}p{{resultIndex}} = GetEnd(p{{firstIndex}}, PTNodeType_{{name}});
{{/nonTerminal_GetEnd}}

{{#nonTerminal_Traverse}}
{{>var}}p{{resultIndex}} = Traverse_{{name}}(p{{firstIndex}}, v);
{{/nonTerminal_Traverse}}

{{#nonTerminal_Parse}}
{{>var}}p{{resultIndex}} = Parse_{{name}}(p{{firstIndex}});
{{/nonTerminal_Parse}}

{{#range}}
{{>var}}p{{resultIndex}} = ParseRange('{{first}}', '{{last}}', p{{firstIndex}});
{{/range}}

{{#char}}
{{>var}}p{{resultIndex}} = ParseChar('{{value}}', p{{firstIndex}});
{{/char}}

{{#dot}}
{{>var}}p{{resultIndex}} = ParseAnyChar(p{{firstIndex}});
{{/dot}}
