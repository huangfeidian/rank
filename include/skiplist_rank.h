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
		static const std::uint32_t MAX_LEVEL = 24;

		struct node
		{

		public:
			rank_info *const rank_info_ptr;
			const std::uint32_t m_level : 31;
			const std::uint32_t m_own_rank_ptr : 1;
			std::array<std::uint32_t, MAX_LEVEL> spans; // 记录当前(this, nexts[i])之间的0级节点数量
			std::array<node *, MAX_LEVEL> nexts;

		public:
			node(rank_info *in_rank_info, std::uint32_t in_level, bool should_delete = true)
				: rank_info_ptr(in_rank_info)
				, m_level(in_level)
				, m_own_rank_ptr(should_delete)
			{
				clear();
			}

			void clear()
			{
				//std::fill(nexts.begin(), nexts.end(), nullptr);
				//std::fill(spans.begin(), spans.end(), 0);
			}
			bool operator<(const node &other) const
			{
				return rank_info_ptr_wrapper{rank_info_ptr} < rank_info_ptr_wrapper{other.rank_info_ptr};
			}
			bool operator<(double other_value) const
			{
				return *rank_info_ptr < other_value;
			}

			~node()
			{
				if (m_own_rank_ptr)
				{
					delete rank_info_ptr;
				}
			}
		};

	private:
		std::unordered_map<std::string, std::unique_ptr<node>> m_key_to_node;
		rank_info m_max_rank_info;
		rank_info m_min_rank_info;

		node m_max_node;
		node m_min_node;
		int m_level; // 最大值为max_level - 1
		std::minstd_rand  m_random_engine;
		std::uniform_int_distribution<std::uint64_t> m_random_dis;

	public:
		skiplist_rank(const std::string& name, std::uint32_t rank_sz, std::uint32_t pool_sz, double min_value, double max_value);

		~skiplist_rank();
		void debug_print() const;
	public:
		// interfaces
		std::uint32_t update(const rank_info& one_player) override;

		std::uint32_t size() const override;

		bool remove(const std::string& key) override;

		std::pair<const rank_info*, std::uint32_t> get_rank(const std::string& key) const override;

		std::uint32_t get_rank(double value) const override;

		const rank_info* get_player(std::uint32_t in_rank) const override;

		
		std::vector<const rank_info*> get_players(std::uint32_t in_begin_rank, std::uint32_t in_end_rank) const override;

		json encode() const override;

		bool decode(const json& data) override;
		bool update_player_info(const std::string& player_id, const json::object_t& player_info) override;
		std::string rank_impl_name() const override
		{
			return "skiplist_rank";
		}
	private:
		int random_level();

		node *create_node(const rank_info &one_player);

		node *find_node(const std::string &key) const;

		// 记录每一层中当前node 的prev节点
		void get_prev_nodes(const node &in_node, std::array<node *, MAX_LEVEL> &prev_nodes, std::array<std::uint32_t, MAX_LEVEL> &prev_ranks);

		// 记录每一层中当前node 的prev节点
		void get_prev_nodes_const(const node &in_node, std::array<const node *, MAX_LEVEL> &prev_nodes, std::array<std::uint32_t, MAX_LEVEL> &prev_ranks) const;

		void remove_from_list(node *cur_node);


		int level() const;
		void shrink_to_pool_sz();
		

	private:
		node *get_node_for_rank(std::uint32_t in_rank) const;

	};
}
