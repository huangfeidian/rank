#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <deque>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <optional>

#include "rank_info.h"

using json = nlohmann::json;
namespace spiritsaway::system::rank
{
	

	class rank_list
	{
		

		std::vector<rank_info> m_rank_info_pool;
		std::vector<std::uint32_t> m_pool_unused_indexes;
		std::unordered_map<std::string, std::uint32_t> m_player_to_pool_idx;
		// 从大到小排序
		std::vector<rank_info_ptr_wrapper> m_sorted_rank_ptrs;
		
	public:
		const std::string m_name;
		const std::uint32_t m_rank_sz;
		const std::uint32_t m_pool_sz;
		enum class update_result
		{
			not_in_rank = 0,
			add_to_rank,
			remain_in_rank
		};
		rank_list(const std::string& name, const std::uint32_t rank_sz, const std::uint32_t pool_sz);
		std::optional<std::uint32_t> rank(const std::string& player_id);
		const std::vector<rank_info_ptr_wrapper>& sorted_rank_ptrs() const
		{
			return m_sorted_rank_ptrs;
		}
		const rank_info* get_rank_info(const std::string& player_id) const
		{
			auto temp_iter = m_player_to_pool_idx.find(player_id);
			if(temp_iter == m_player_to_pool_idx.end())
			{
				return nullptr;
			}
			else
			{
				return &m_rank_info_pool[temp_iter->second];
			}
		}
		bool update_player_info(const std::string& player_id, const json::object_t& player_info);
		// player_ranks 从大到小排序
		void reset(const std::vector<rank_info>& player_ranks);
		bool remove(const std::string& player_id);
		std::uint32_t size() const
		{
			return m_sorted_rank_ptrs.size();
		}
		update_result update(const rank_info& one_player);
		json encode() const;
		static std::unique_ptr<rank_list> create(const json& data);

	};
}