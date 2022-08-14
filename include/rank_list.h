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
		
		const std::string m_name;
		const std::uint32_t m_rank_sz;
		const std::uint32_t m_pool_sz;
		std::unordered_map<std::string, std::unique_ptr<rank_info>> m_player_rank_infos;
		// 从大到小排序
		std::vector<rank_info_ptr_wrapper> m_sorted_player_keys;
		
	public:
		enum class update_result
		{
			not_in_rank = 0,
			add_to_rank,
			remain_in_rank
		};
		rank_list(const std::string& name, const std::uint32_t rank_sz, const std::uint32_t pool_sz);
		std::optional<std::uint32_t> rank(const std::string& player_id);
		// player_ranks 从大到小排序
		void reset(const std::vector<rank_info>& player_ranks);
		bool remove(const std::string& player_id);
		update_result update(const rank_info& one_player);
		json encode() const;
		static std::unique_ptr<rank_list> create(const json& data);

	};
}