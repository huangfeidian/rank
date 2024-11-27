#pragma once

#include "rank_info.h"
#include <optional>
namespace spiritsaway::system::rank
{
	class rank_interface
	{
	public:
		rank_interface();
		virtual ~rank_interface();
		virtual json encode() const = 0;
		virtual bool decode(const json& data) = 0;

		virtual bool update_player_info(const std::string& player_id, const json::object_t& player_info) = 0;
		virtual bool remove(const std::string& player_id) = 0;
		virtual std::uint32_t size() const = 0;
		// rank 为1 代表第一名  为0 代表不在排行榜上
		
		virtual std::uint32_t update(const rank_info& one_player) = 0;
		// 获取玩家对应的信息与排名
		virtual std::pair<const rank_info*, std::uint32_t> get_rank(const std::string& player_id) const = 0;
		virtual const rank_info* get_rank(std::uint32_t in_rank) const = 0;
		// 获取这个分数代表的同分最后一名的排名 
		virtual std::uint32_t get_rank(double rank_value) const = 0;
	};
}