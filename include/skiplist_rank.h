#pragma once

#include <cstddef>
#include <cassert>
#include <ctime>
#include <random>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include <iostream>
#include "rank_interface.h"
namespace spiritsaway::system::rank
{

	class skiplist_rank : public rank_interface
	{
		static const std::uint32_t MAX_LEVEL = 16;

		struct node
		{

		public:
			rank_info *const rank_info_ptr;
			std::array<std::uint32_t, MAX_LEVEL> spans; // 记录当前(this, nexts[i])之间的0级节点数量
			std::array<node *, MAX_LEVEL> nexts;

		public:
			node(rank_info *in_rank_info)
				: rank_info_ptr(in_rank_info)
			{
				clear();
			}

			void clear()
			{
				std::fill(nexts.begin(), nexts.end(), nullptr);
				std::fill(spans.begin(), spans.end(), 0);
			}
			bool operator<(const node &other) const
			{
				return rank_info_ptr_wrapper{rank_info_ptr} < rank_info_ptr_wrapper{other.rank_info_ptr};
			}
			bool operator<(double other_value) const
			{
				return *rank_info_ptr < other_value;
			}
		};

	private:
		std::unordered_map<std::string, std::unique_ptr<node>> m_key_to_node;
		rank_info m_max_rank_info;
		rank_info m_min_rank_info;

		node m_max_node;
		node m_min_node;
		int m_level; // 最大值为max_level
		std::uint64_t m_seq_counter;
		std::default_random_engine m_random_engine;
		std::uniform_int_distribution<std::uint64_t> m_random_dis;

	public:
		skiplist_rank(double in_min_value, double in_max_value);

		~skiplist_rank();

	private:
		int random_level();

		node *create_node(const rank_info &one_player);

		node *find_node(const std::string &key) const;

		// 记录每一层中当前node 的prev节点
		void get_prev_nodes(const node &in_node, std::array<node *, MAX_LEVEL> &prev_nodes, std::array<std::uint32_t, MAX_LEVEL> &prev_ranks);

		// 记录每一层中当前node 的prev节点
		void get_prev_nodes_const(const node &in_node, std::array<const node *, MAX_LEVEL> &prev_nodes, std::array<std::uint32_t, MAX_LEVEL> &prev_ranks) const;

		void remove_from_list(node *cur_node);

		// 返回插入之后的排名
		std::uint32_t update(const rank_info &one_player);

		bool remove(const std::string& key);

		int level() const
		{
			return m_level;
		}

		std::uint32_t size() const
		{
			return m_key_to_node.size();
		}

		// 排名从1 开始算 最左边的有效节点为第一名
		// 0 代表不在排名里
		std::uint32_t get_rank_for_key(const std::string &key) const;

		// 有多个相同value时 返回最后一个value的排名
		std::uint32_t get_rank_for_value(double value) const;

		const rank_info *get_key_for_rank(std::uint32_t in_rank) const;

		// 获取[in_begin_rank, in_end_rank]之间的节点
		std::vector<const rank_info *> get_keys_in_rank_range(std::uint32_t in_begin_rank, std::uint32_t in_end_rank) const;

		json encode() const;

		bool decode(const json &data);

	private:
		const node *get_node_for_rank(std::uint32_t in_rank) const;
	};
}
