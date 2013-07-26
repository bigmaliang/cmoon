Type    ID

Double	1
String	2
Object	3
Array	4
Binary data	5
Object id	7
Boolean	8
Date	9
Null	10
Regular expression	11
JavaScript code	13
Symbol	14
JavaScript code with scope	15
32-bit integer	16
Timestamp	17
64-bit integer	18

db.pk_history.find({vote_history_id: {$type: 2}})
db.test.find({"key":{$type:2}}).forEach(function(x) {
 x.key= new NumberInt(x.key);
 db.test.save(x);
});


// 获取自增id
db.system.js.save({
    _id: 'counter',
    value: function(name)
    {
        var ret = db.count.findAndModify({
            query:{_id:name},
            update:{$inc : {id:1}},
            "new":true,
            upsert:true
        });
        
        return ret.id;
    }
})


// 统计投票结果
db.system.js.save({
    _id: 'quizRate',
    value: function(id, item)
    {
        var total = db.quiz_history.find({'obj_id': id}).count();
        var count = db.quiz_history.find({'obj_id': id, 'item': item}).count();

        if (total > 0) return Math.round(count / total) * 100;
        else return 0;
    }
})

db.system.js.save({
    _id: 'baker',
    value: function(collname)
    {
        var col = db.getCollection(collname);
        
        col.renameCollection('baker_tmp');
        db.baker_tmp.find().forEach(function(x) {
            db.getCollection('archive_'+collname).save(x);
        });
        db.baker_tmp.drop();
        //if (collname == 'aoicropperday') {
        //    db.aoicropperday.renameCollection('aoicropperday_tmp');
        //    db.aoicropperday_tmp.find().forEach(function(x) {
        //        db.aoicropperday_archive.save(x);
        //    });
        //    db.aoicropperday_tmp.drop();
        //}
    }
})

db.vote_history.find({}).forEach(function(x) {
    x.vote_history_id = parseInt(x.vote_history_id);
    db.vote_history.save(x);
});

db.vote_history.renameCollection('guess_history')
db.sysload.insert({'obj_id': 1, 'systembusy': 0})
db.advertise.insert({'obj_id':1, is_show: 1, pics: {'iosurl': 'advertise/iOS-960-640.png', 'androidurl': 'advertise/android-1280-720.png'}})
db.count._id => db.count.coll_name

db.notice.find({}).forEach(function(x) {if (x.obj_id == 97 && !x.content.match(/开奖即将开始，赶紧瞧瞧去/)) echo "xxx";})

db.notice.find({}).forEach(function(x) {if (x.obj_id == 97) echo "xxx";})

// 查看custom command 格式
db.listCommands()
