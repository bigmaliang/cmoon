-- reqtype 0: html, 1: ajax
-- lmttype 0: none, 1: LMT_TYPE_MEMBER, 2: LMT_TYPE_GJOIN, 3 LMT_TYPE_JUNIOR, 4 LMT_TYPE_SENIOR, 5 LMT_TYPE_GADM, 6 LMT_TYPE_GOWN, 255: LMT_TYPE_ROOT
-- 1111 15
-- 0001' 0000'0001' 0000'0000 65792
-- 0101' 0000'0001' 0000'0001 327937
-- 1111' 0000'0111' 0000'0001 984833
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (2, 1, 1001, 65792, 0, 0, 'admin', '管理'); -- can't direct access, 2
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (3, 1, 1001, 1, 0, 0, 'member', '用户'); -- can't direct access, 3
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (4, 1, 1001, 1, 0, 0, 'service', '服务'); -- can't direct access, 4
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (5, 1, 1001, 327937, 0, 0, 'csc', '菜市场'); -- 5

INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (2, 2, 1001, 15, 1, 1, 'profile', '信息');
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (2, 2, 1001, 15, 1, 255, 'account', '帐号');
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (2, 2, 1001, 15, 1, 4, 'file', '文件');
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (2, 2, 1001, 15, 1, 3, 'group', '组员');
UPDATE fileinfo SET dataer='admin_profile' where uri='/admin/profile';
UPDATE fileinfo SET dataer='admin_account' where uri='/admin/account';
UPDATE fileinfo SET dataer='admin_file' where uri='/admin/file';
UPDATE fileinfo SET dataer='admin_group' where uri='/admin/group';

INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (3, 3, 1001, 1, 1, 0, 'login', '登录');
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (3, 3, 1001, 1, 1, 0, 'logout', '登出');
INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (3, 3, 1001, 1, 1, 0, 'regist', '注册');
UPDATE fileinfo SET dataer='member_login' where uri='/member/login';
UPDATE fileinfo SET dataer='member_logout' where uri='/member/logout';
UPDATE fileinfo SET dataer='member_regist' where uri='/member/regist';

INSERT INTO fileinfo (aid, pid, uid, mode, reqtype, lmttype, name, remark) VALUES (4, 4, 1001, 65792, 1, 0, 'action', '动作');
UPDATE fileinfo SET dataer='service_action' where uri='/service/action';

-- dataer, render 'll be modified by trigger after row inserted, so we modify them apartly
UPDATE fileinfo SET dataer='tjt', render='tjt' WHERE uri='/csc';
