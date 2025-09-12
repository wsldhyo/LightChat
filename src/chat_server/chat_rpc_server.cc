#include "chat_rpc_server.hpp"
#include "utility/userinfo.hpp"
ChatServiceImpl::ChatServiceImpl()
{

}

Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request,
	AddFriendRsp* reply) {
	return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context,
	const AuthFriendReq* request, AuthFriendRsp* response) {
	return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context,
	const TextChatMsgReq* request, TextChatMsgRsp* response) {
	return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
	return true;
}