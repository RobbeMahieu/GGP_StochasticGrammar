#pragma once
#include <vector>
#include <string>

class Node {
	public:
		Node();
		virtual ~Node() = default;

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

		virtual std::string Parse() = 0;
};


class LeafNode : public Node
{
	public:
		LeafNode(std::string value);
		virtual ~LeafNode() = default;

		LeafNode(const LeafNode&) = delete;
		LeafNode(LeafNode&&) = delete;
		LeafNode& operator=(const LeafNode&) = delete;
		LeafNode& operator=(LeafNode&&) = delete;

		virtual std::string Parse() override;

	private:
		std::string m_Value;
};

class SelectNode : public Node
{
	public:
		SelectNode();
		virtual ~SelectNode() = default;

		SelectNode(const SelectNode&) = delete;
		SelectNode(SelectNode&&) = delete;
		SelectNode& operator=(const SelectNode&) = delete;
		SelectNode& operator=(SelectNode&&) = delete;

		virtual std::string Parse() override;
		void AddOption(Node* option);

	private:
		std::string m_Value;
		std::vector<Node*> m_pOptions;
};