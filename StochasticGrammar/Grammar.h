#pragma once
#include <string>
#include "Nodes.h"
#include <memory>
#include <unordered_map>

#define SEQ_DEL " & "
#define SEL_DEL " | "
#define REP_DEL " # "
#define LND_DEL " -> "

using weightedRule = std::pair<std::string, float>;
using repeatedRule = std::pair<std::string, float>;

class Rule404Exception {};

template<typename Data>
class Grammar
{
	public:
		Grammar();
		virtual ~Grammar() = default;

		Grammar(const Grammar&) = delete;
		Grammar(Grammar&&) = delete;
		Grammar& operator=(const Grammar&) = delete;
		Grammar& operator=(Grammar&&) = delete;

		std::vector<Data> GenerateSequence(std::string rule);

		void ParseRule(const std::string& name, const std::string& rule);
		void AddLeaveNode(const std::string& name, const Data& data);

	private:
		std::unordered_map<std::string,std::shared_ptr<Node<Data>>> m_pRules;

		void ParseSelectorRule(const std::string& name, std::string& rule);
		void ParseSequenceRule(const std::string& name, std::string& rule);
		void ParseRepetitionRule(const std::string& name, std::string& rule);
		void ParseLNodeRule(const std::string& name, std::string& rule);
		void ParseSingleRule(const std::string& name, std::string rule);

		void ChangeRule(const std::string& ruleName, std::shared_ptr<Node<Data>> newNode);

		void AddSingleRule(const std::string& name, const std::string& rule);
		void AddSelectorRule(const std::string& name, const std::vector<weightedRule> rules);
		void AddSequenceRule(const std::string& name, const std::vector<std::string> rules);
		void AddRepetitionRule(const std::string& name, const repeatedRule rule);
		void AddLNodeRule(const std::string& name, const std::string& rule, const std::string& fallbackRule);
};

template<typename Data>
Grammar<Data>::Grammar()
{}

template<typename Data>
std::vector<Data> Grammar<Data>::GenerateSequence(std::string ruleName) {

	if (m_pRules.find(ruleName) == m_pRules.end()) {
		throw Rule404Exception{};
	}

	std::vector<Data> result{};
	m_pRules[ruleName]->Parse(result, 0);
	return result;
}

template<>
void Grammar<std::string>::AddSingleRule(const std::string& name, const std::string& rule) {

	// Non-existing subrule
	if (m_pRules.find(rule) == m_pRules.end()) {
		m_pRules[rule] = std::make_shared<LeafNode<std::string>>(rule);
	}

	if (m_pRules.find(name) != m_pRules.end()) {
		ChangeRule(name, m_pRules[rule]);
		return;
	}

	// Non-existing rule
	m_pRules[name] = m_pRules[rule];
}

template<typename Data>
void Grammar<Data>::AddSingleRule(const std::string& name, const std::string& rule) {

	// Non-existing subrule
	if (m_pRules.find(rule) == m_pRules.end()) {

		throw Rule404Exception{};
	}

	if (m_pRules.find(name) != m_pRules.end()) {
		ChangeRule(name, m_pRules[rule]);
		return;
	}

	// Non-existing rule
	m_pRules[name] = m_pRules[rule];
}

template<typename Data>
void Grammar<Data>::AddLeaveNode(const std::string& name, const Data& data) {
	std::shared_ptr<LeafNode<Data>> leaf{ std::make_shared<LeafNode<Data>>(data) };
	m_pRules[name] = leaf;
}

template<typename Data>
void Grammar<Data>::AddSelectorRule(const std::string& name, const std::vector<weightedRule> rules) {
	std::shared_ptr<SelectNode<Data>> selectNode = std::make_shared<SelectNode<Data>>();
	for (auto& rule : rules) {

		// Non-existing subrule
		if (m_pRules.find(rule.first) == m_pRules.end()) {
			ParseRule(rule.first, rule.first);
		}

		// Add to selectNode
		selectNode->AddOption(m_pRules[rule.first].get(), rule.second);
	}

	if (m_pRules.find(name) != m_pRules.end()) {
		ChangeRule(name, selectNode);
		return;
	}

	// Non-existing rule
	m_pRules[name] = selectNode;
}

template<typename Data>
void Grammar<Data>::AddSequenceRule(const std::string& name, const std::vector<std::string> rules) {
	std::shared_ptr<SequenceNode<Data>> sequenceNode = std::make_shared<SequenceNode<Data>>();
	for (auto& rule : rules) {

		// Non-existing subule
		if (m_pRules.find(rule) == m_pRules.end()) {
			ParseRule(rule, rule);
		}

		// Add to selectNode
		sequenceNode->AddElement(m_pRules[rule].get());
	}

	if (m_pRules.find(name) != m_pRules.end()) {
		ChangeRule(name, sequenceNode);
		return;
	}

	// Non-existing rule
	m_pRules[name] = sequenceNode;
}

template<typename Data>
void Grammar<Data>::AddRepetitionRule(const std::string& name, const repeatedRule rule) {

	// Non-existing subrule
	if (m_pRules.find(rule.first) == m_pRules.end()) {
		ParseRule(rule.first, rule.first);
	}

	if (m_pRules.find(name) != m_pRules.end()) {
		ChangeRule(name, std::make_shared<RepetitionNode<Data>>(m_pRules[rule.first].get(), rule.second));
		return;
	}

	// Non-existing rule
	m_pRules[name] = std::make_shared<RepetitionNode<Data>>(m_pRules[rule.first].get(), rule.second);
}

template<typename Data>
void Grammar<Data>::AddLNodeRule(const std::string& name, const std::string& rule, const std::string& fallbackRule) {
	
	// Non-existing subrule
	if (m_pRules.find(rule) == m_pRules.end()) {
		ParseRule(rule, rule);
	}
	if (m_pRules.find(fallbackRule) == m_pRules.end()) {
		ParseRule(fallbackRule, fallbackRule);
	}

	if (m_pRules.find(name) != m_pRules.end()) {
		ChangeRule(name, std::make_shared<LNode<Data>>(m_pRules[rule].get(), m_pRules[fallbackRule]));
		return;
	}

	// Non-existing rule
	m_pRules[name] = std::make_shared<LNode<Data>>(m_pRules[rule].get(), m_pRules[fallbackRule]);
}

template<typename Data>
void Grammar<Data>::ChangeRule(const std::string& ruleName, std::shared_ptr<Node<Data>> newNode) {

	// Get the old node
	std::shared_ptr<Node<Data>> oldNode = m_pRules[ruleName];
	if (oldNode == newNode) {
		return;
	}


	// Update the rule
	m_pRules[ruleName] = newNode;

	// Change every reference to the old node to the new one
	for (auto& rule : m_pRules) {
		rule.second->SwapDependingNode(oldNode.get(), newNode.get());
	}	
}


// Rule syntax:
// Leafnodes are also considered rules
// Each string refers to another rule
// Rules need to be seperated by an operator:
//		sequence: [rule] & [rule]
//		selector: [weight] [rule] | [weight] [rule]
//		repetiton: [rule] # [times]

template<typename Data>
void Grammar<Data>::ParseRule(const std::string& name, const std::string& rule) {

	std::string parsedRule{ rule };

	if (parsedRule.find(LND_DEL) != std::string::npos) {
		ParseLNodeRule(name, parsedRule);
		return;
	}
	if (parsedRule.find(SEQ_DEL) != std::string::npos) {
		ParseSequenceRule(name, parsedRule);
		return;
	}
	if (parsedRule.find(SEL_DEL) != std::string::npos) {
		ParseSelectorRule(name, parsedRule);
		return;
	}
	if (parsedRule.find(REP_DEL) != std::string::npos) {
		ParseRepetitionRule(name, parsedRule);
		return;
	}
	ParseSingleRule(name, rule);
}

template<typename Data>
void Grammar<Data>::ParseSelectorRule(const std::string& name, std::string& rule) {

	//split into subrules
	std::vector<std::string> rules;
	size_t first;
	size_t last = 0;

	while ((first = rule.find_first_not_of(SEL_DEL, last)) != std::string::npos) {
		last = rule.find(SEL_DEL, first);
		rules.push_back(rule.substr(first, last - first));
	}

	// turn to weighted rule
	std::vector<weightedRule> weightedRules;
	for (const std::string& rule : rules) {

		size_t spaceIndex = rule.find(" ");
		std::string weightString{ rule.substr(0, spaceIndex) };
		std::string ruleName{ rule.substr(spaceIndex + 1, rule.length() - weightString.length() - 1) };

		float weight{ std::stof(weightString) };
		weightedRules.push_back(std::make_pair(ruleName, weight));
	}

	// Add to grammar
	AddSelectorRule(name, weightedRules);
}

template<typename Data>
void Grammar<Data>::ParseSequenceRule(const std::string& name, std::string& rule) {

	//split into subrules
	std::vector<std::string> rules;
	size_t first;
	size_t last = 0;

	while ((first = rule.find_first_not_of(SEQ_DEL, last)) != std::string::npos) {
		last = rule.find(SEQ_DEL, first);
		rules.push_back(rule.substr(first, last - first));
	}

	AddSequenceRule(name, rules);
}

template<typename Data>
void Grammar<Data>::ParseSingleRule(const std::string& name, std::string rule) {
	AddSingleRule(name, rule);
}

template<typename Data>
void Grammar<Data>::ParseRepetitionRule(const std::string& name, std::string& rule) {

	std::string del{ REP_DEL };
	size_t splitIndex = rule.find(del);
	std::string ruleName{ rule.substr(0, splitIndex) };
	std::string repetitionString{ rule.substr(splitIndex + del.length(), rule.length() - ruleName.length() - del.length())};

	float repetitions{ std::stof(repetitionString) };
	AddRepetitionRule(name, std::make_pair(ruleName, repetitions));
}

template<typename Data>
void Grammar<Data>::ParseLNodeRule(const std::string& name, std::string& rule) {

	std::string del{ LND_DEL };
	size_t splitIndex = rule.find(del);
	std::string fallbackName{ rule.substr(0, splitIndex) };
	std::string ruleName{ rule.substr(splitIndex + del.length(), rule.length() - ruleName.length() - del.length()) };

	AddLNodeRule(name, ruleName, fallbackName);
}